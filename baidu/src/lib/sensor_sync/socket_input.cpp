/************************************************************************
 *
 * Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
 *
 ************************************************************************/

/*
 * @file   socket_input.cc
 * @author fengkaiwen01@ (original),
 *         feiaiguo@ (completely re-design/implementation)
 *         youxiangtao@ (May 2018)
 * @brief
 */

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <string.h>

#include "sensor_sync.h"

namespace velodyne_driver {

UdpRcvSocket::~UdpRcvSocket()
{
    if (_sockfd != -1) {
        close(_sockfd);
    }
}

bool UdpRcvSocket::init_socket()
{
    if (_sockfd != -1) {
        close(_sockfd);
    }

    // connect to Velodyne UDP port
    VELO_LOG_INFO("Opening UDP socket: port %d\n", _port);
    _sockfd = socket(PF_INET, SOCK_DGRAM, 0);

    if (_sockfd == -1) {
        VELO_LOG_ERR("Failed to create UDP socket: port %d\n", _port);
        return false;
    }

    // allow multiple sockets to use the same PORT number
    int optval = 1;
    (void) setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;  // host byte order
    my_addr.sin_port = htons(_port);   // short, in network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY;  // automatically fill in my IP

    if (bind(_sockfd, (sockaddr*) &my_addr, sizeof(sockaddr)) == -1) {
        VELO_LOG_ERR("Socket bind failed!\n");
        return false;
    }

    if (fcntl(_sockfd, F_SETFL, O_NONBLOCK | FASYNC) < 0) {
        VELO_LOG_ERR("failed to set socket non-block via fcntl\n");
        return false;
    }

    VELO_LOG_DEBUG("Receiving socket fd is %d, for port %d\n", _sockfd, _port);
    return true;
}

int UdpRcvSocket::get_next_packet_nowait(void *buf, int size)
{
    ssize_t nbytes = recvfrom(_sockfd, buf, size, 0, NULL, NULL);
	VELO_LOG_DEBUG("Got %d, from port %d, bufersize = %d, nowait\n",
				(int)nbytes,(int)_port, size);

    if (nbytes < 0) {  // No data.
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available at this time.
            return Input::NO_INPUT;
        } else {  // Don't know what's going on, can't recover.
            VELO_LOG_ERR("recvfail\n");
            return Input::FATAL_ERR;
        }
    }

    if ((int)nbytes == size) {
        ++_stats.rcv_cnt;
        return Input::OK;
    }

    VELO_LOG_DEBUG("Packet size not expected (bytes): got %d, expect: %d\n",
        (int)nbytes, size);

    ++_stats.sz_mismatch_cnt;
    return Input::ERROR;
}

int UdpRcvSocket::get_next_packet(void *buf, int size, int timeout_ms)
{
    while (true) {
        int has_input = input_available(timeout_ms);
        if (has_input < 0) {
            return has_input;
        }

        ssize_t nbytes = recvfrom(_sockfd, buf, size, 0, NULL, NULL);

		VELO_LOG_DEBUG("Got %d, from port %d, bufersize = %d, timeout = %d\n",
				(int)nbytes,(int)_port, size, timeout_ms);

        if (nbytes < 0) {  // No data.
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Data gone away between poll() and recvfrom(), try again.
                ++_stats.err_cnt;
                return Input::ERROR;
            } else {  // Don't know what's going on, can't recover.
                VELO_LOG_ERR("recvfail");
                return Input::FATAL_ERR;
            }
        }

        if ((int) nbytes == size) {
            // fix-me: if actual nbytes > size, it would still match here.
            // We can only detect that if we use a larger buffer.
            // Again, we never checked if data received is in the right format.
            // We need to make sure only Lidar data is coming to the given port.
            ++_stats.rcv_cnt;
            return Input::OK;
        }

        VELO_LOG_DEBUG("Packet size not expected (bytes): got %d, expect: %d\n",
            (int)nbytes, size);

        ++_stats.sz_mismatch_cnt;
        return Input::ERROR;
    }
}

int UdpRcvSocket::input_available(int timeout_ms)
{
    struct pollfd fds[1];
    fds[0].fd = _sockfd;
    fds[0].events = POLLIN;

    // Unfortunately, the Linux kernel recvfrom() implementation
    // uses a non-interruptible sleep() when waiting for data,
    // which would cause this method to hang if the device is not
    // providing data.  We poll() the device first to make sure
    // there is data, then we will do a nonblocking recvfrom().

    int retval = poll(fds, 1, timeout_ms);

    if (retval > 0) {
        if (fds[0].revents & POLLIN) {
            return Input::OK;
        }
        if ((fds[0].revents & POLLERR) || (fds[0].revents & POLLHUP)
                || (fds[0].revents & POLLNVAL)) {  // network error, device error
            VELO_LOG_ERR("poll() error: %d\n", fds[0].revents);
            return Input::FATAL_ERR;
        }
    }

    if (retval == 0) {  // poll() timeout?
        VELO_LOG_DEBUG("Velodyne poll() timeout\n");
        ++_stats.timeout_cnt;
        return Input::POLL_TIMEOUT;
    }

    // must be (retval < 0) if we get here, poll() error?
    if (errno == EINTR) {  // signal
        ++_stats.err_cnt;
        return Input::ERROR;
    } else {
        VELO_LOG_ERR("Poll() failed: %s\n", strerror(errno));
        return Input::FATAL_ERR;
    }
}

void UdpRcvSocket::drain_data()
{
    char buf[MAX_BUF_SZ];
    while (true) {
        switch (get_next_packet_nowait(buf, MAX_BUF_SZ)) {
        case Input::OK:
            continue;
        default:
            break;
        }
    }
}

int SocketInput::get_firing_data_packet(bool block)
{
    int status;
	VELO_LOG_DEBUG("%s@%d\n", __func__, __LINE__);
    if (block) {
        status = _rcv_sock->get_next_packet(data_buf(), _size, _timeout_ms);
    } else {
        status = _rcv_sock->get_next_packet_nowait(data_buf(), _size);
    }

    return status;
}

int SocketInput::loop_until_gpstime(VeloProc *velo_proc, int loops)
{
    if (velo_proc->got_gpstime()) {
        _rcv_sock->drain_data();
        return Input::OK;
    }

    int count = 0;
    while (count++ < loops) {
        switch (get_firing_data_packet(true)) {
        case Input::FATAL_ERR:
            return Input::FATAL_ERR;
        case Input::OK:
            velo_proc->proc_firing_data(data_buf(), pos_buf());
            if (velo_proc->got_gpstime()) {
                // Need to get rid of all firing data queued, otherwise we get
                // stale data when we start to read firing data.
                _rcv_sock->drain_data();
                return Input::OK;
            }
        default:
            // Poll time-out or recoverable error.
            continue;
        }
    }
    return Input::ERROR;
}

int SocketInput64e::get_firing_data_packet(bool UNUSED(block))
{
	VELO_LOG_DEBUG("%s@%d\n",__func__, __LINE__);
    return _rcv_sock->get_next_packet(data_buf(), _size, _timeout_ms);
}

int SocketInput32eVlp16::get_firing_data_packet(bool UNUSED(block))
{
	VELO_LOG_DEBUG("%s@%d\n",__func__, __LINE__);
    int status1 = _data_input->get_firing_data_packet(false);
    if (status1 == Input::FATAL_ERR) {
        return status1;
    }

	VELO_LOG_DEBUG("%s@%d\n",__func__, __LINE__);
    int status2 = _pos_input->get_firing_data_packet(false);
    if (status2 == Input::FATAL_ERR) {
        return status2;
    }
	VELO_LOG_DEBUG("%s@%d\n",__func__, __LINE__);
    if (status1 != Input::OK) {
        status1 = _data_input->get_firing_data_packet(true);
    }

    return status1;
}

int SocketInput32eVlp16::loop_until_gpstime(VeloProc *velo_proc, int loops)
{
    return _pos_input->loop_until_gpstime(velo_proc, loops);
}

} // velodyne namespace
