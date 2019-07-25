/************************************************************************
 *
 * Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
 *
 ************************************************************************/

/*
 * @file   socket_input.h
 * @author fengkaiwen01@ (original),
 *         feiaiguo@ (completely re-design/implementation)
 *         youxiangtao@ (May 2018)
 * @brief
 */

#ifndef VELODYNE_DRIVER_SOCKET_INPUT_H
#define VELODYNE_DRIVER_SOCKET_INPUT_H

#include <memory>

#include "input.h"

namespace velodyne_driver {

static const int FIRING_DATA_PORT = 2368;
//For HDL32E and VPL16
static const int POSITIONING_DATA_PORT = 8308;

static const int MAX_BUF_SZ = 1480;

static const int DEFAULT_POLL_TIMEOUT_MS = 10; // 10 milli-sec

class UdpRcvSocket {

public:

    explicit UdpRcvSocket(uint16_t port) :
        _sockfd(-1), _port(port) {}

    ~UdpRcvSocket();

    virtual bool init_socket();

    /*
     * @brief Gets the next packet from socket, don't wait.
     * @return one of the following:
     *  Input::OK,
     *  Input::NO_INPUT,
     *  Input::ERROR,
     *  Input::FATAL_ERR.
     */
    int get_next_packet_nowait(void *buf, int size);

    /*
     * @brief Gets the next packet from socket.
     * @return one of the following:
     *  Input::OK,
     *  Input::POLL_TIMEOUT,
     *  Input::ERROR,
     *  Input::FATAL_ERR.
     */
    int get_next_packet(void *buf, int size, int timeout_ms);

    void drain_data();

    void log_stats()
    {
        VELO_LOG_INFO("    port %d: ", _port);
        _stats.log();
    }

private:
    int _sockfd;
    uint16_t _port;
    int input_available(int timeout_ms);

    Input::Stats _stats;

    DISALLOW_COPY_AND_ASSIGN(UdpRcvSocket);
};

/** @brief Live Velodyne input from socket. */
class SocketInput: public Input {

public:
    SocketInput(uint16_t port, int timeout_ms, int size) :
        _rcv_sock(new UdpRcvSocket(port)),
        _timeout_ms(timeout_ms),
        _size(size)
    {
        _data.reset(new uint8_t[size]);
    }

    virtual ~SocketInput() {}

    virtual bool init()
    {
        return _rcv_sock->init_socket();
    }

    virtual uint8_t *data_buf()
    {
        return _data.get();
    }

    virtual uint8_t *pos_buf()
    {
        return _data.get();
    }

    virtual int get_firing_data_packet(bool block);

    /** @brief Loops through packets until we get GPS time.
     * @param loops # of loops to try
     *  (each loop may wait until a poll time-out value).
     * @returns OK if we get valid GPS timestamp,
     *  ERROR if loop limit reached or end of input reached,
     *  FATAL_ERR if fatal error occured before we get valid GPS timestamp.
     */
    virtual int loop_until_gpstime(VeloProc *velo_proc, int loops);

    virtual void log_stats()
    {
        _rcv_sock->log_stats();
    }

protected:
    std::unique_ptr<UdpRcvSocket> _rcv_sock;
    std::unique_ptr<uint8_t[]> _data;
    int _timeout_ms;
    int _size;

    DISALLOW_COPY_AND_ASSIGN(SocketInput);
};

/** @brief live input from socket, for Velodyne 64E */
class SocketInput64e: public SocketInput {

public:
    SocketInput64e(uint16_t port, int timeout_ms) :
        SocketInput(port, timeout_ms, FIRING_DATA_PACKET_SIZE) {}

    virtual ~SocketInput64e() {}

    int get_firing_data_packet(bool block);

private:
    DISALLOW_COPY_AND_ASSIGN(SocketInput64e);
};

/** @brief Live Velodyne input from socket, including point data with
 * positioning data for 32E & VLP16.
 */
class SocketInput32eVlp16 : public Input {

public:
    SocketInput32eVlp16(uint16_t port, uint16_t pos_port, int timeout_ms, int firing_data_pkt_size, int pos_data_pkt_size) :
        _data_input(new SocketInput(port, timeout_ms, firing_data_pkt_size)),
        _pos_input(new SocketInput(pos_port, timeout_ms, pos_data_pkt_size)) {
			VELO_LOG_DEBUG("%s@%d: data port = %d, pos_port = %d, timeout_ms = %d, firing size = %d, pos data size = %d\n",
					__func__, __LINE__,
					port, pos_port, timeout_ms, firing_data_pkt_size, pos_data_pkt_size);
		}

    virtual ~SocketInput32eVlp16() {}

    bool init()
    {
        return _data_input->init() && _pos_input->init();
    }

    int get_firing_data_packet(bool block);

    int loop_until_gpstime(VeloProc *velo_proc, int loops);

    uint8_t *data_buf()
    {
        return _data_input->data_buf();
    }

    uint8_t *pos_buf()
    {
        return _pos_input->pos_buf();
    }

    virtual void log_stats()
    {
        _data_input->log_stats();
        _pos_input->log_stats();
    }

protected:
    std::unique_ptr<SocketInput> _data_input;
    std::unique_ptr<SocketInput> _pos_input;

    DISALLOW_COPY_AND_ASSIGN(SocketInput32eVlp16);
};

} // velodyne_driver namespace

#endif // VELODYNE_DRIVER_SOCKET_INPUT_H
