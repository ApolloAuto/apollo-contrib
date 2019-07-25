/************************************************************************
 *
 * Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
 * *
 ************************************************************************/

/*
 * @file   input.cc
 * @author fengkaiwen01@ (original), feiaiguo@ (re-design/implementation)
 *         youxiangtao@ (May 2018)
 * @brief
 */

#include <poll.h>
#include <errno.h>
#include <memory.h>
#include <algorithm>

#include "sensor_sync.h"

namespace velodyne_driver {

bool PcapInput::init()
{
    if (_pcap != NULL) {
        pcap_close(_pcap);
    }

    if (_live) {
        VELO_LOG_INFO("Opening net device (%s): "
            "readOnce=%d, readFast=%d, repeatDelay=%0.3fsec, live=%d\n",
            _filename.c_str(), _read_once, _read_fast, _repeat_delay, _live);

        _pcap = pcap_open_live(_filename.c_str(),
            ETHERNET_HEADER_SIZE +
            std::max(_firing_pkt_size, _pos_pkt_size),
            1, 1000, _errbuf);
    } else {
        VELO_LOG_INFO("Opening PCAP file (%s): "
            "readOnce=%d, readFast=%d, repeatDelay=%0.3fsec, live=%d\n",
            _filename.c_str(), _read_once, _read_fast, _repeat_delay, _live);

        _pcap = pcap_open_offline(_filename.c_str(), _errbuf);
    }

    if (_pcap == NULL) {
        VELO_LOG_ERR("Error opening PCAP input! %s.\n", _errbuf);
        return false;
    }

    char filter[32];
    struct bpf_program fp;

    snprintf(filter, sizeof(filter),
        "dst port %u or dst port %u", _port, _pos_port);

    if (pcap_compile(_pcap, &fp, filter, 0, 0xffffffff) == -1) {
        VELO_LOG_INFO("failed to parse filter %s: %s\n",
            filter, pcap_geterr(_pcap));
        return false;
    }

    if (pcap_setfilter(_pcap, &fp) == -1) {
        VELO_LOG_INFO("failed to install filter %s: %s\n",
            filter, pcap_geterr(_pcap));
        return false;
    }

    return true;
}

PcapInput::~PcapInput(void)
{
    if (_pcap) {
        pcap_close(_pcap);
    }
}

int PcapInput::get_firing_data_packet(bool UNUSED(block))
{
    struct pcap_pkthdr* header;
    const u_char* pkt_data;

    while (true) {
        int res = 0;
        if ((res = pcap_next_ex(_pcap, &header, &pkt_data)) >= 0) {
            if (!filter_firing_packet(header, pkt_data)) {
                continue;
            }

            // Keep the reader from blowing through the file.
            if (!_live && (_read_fast == false)) {
                _packet_rate.sleep();
            }

            _empty = false;
            return 0;                   // success
        }

        if (_empty) {                // no data in file?
            VELO_LOG_WARN("Error %d reading Velodyne packet: %s\n", res,
                    pcap_geterr(_pcap));
            return FATAL_ERR;
        }

        if (_read_once) {
            VELO_LOG_INFO("end of file reached -- done reading.\n");
            return END;
        }

        if (_repeat_delay > 0.0) {
            VELO_LOG_INFO("end of file reached -- delaying %.3f seconds.\n",
                    _repeat_delay);
            usleep(int(_repeat_delay * 1000000));
        }

        VELO_LOG_INFO("replaying Velodyne dump file");

        // I can't figure out how to rewind the file, because it
        // starts with some kind of header.  So, close the file
        // and reopen it with pcap.
        pcap_close(_pcap);
        _pcap = pcap_open_offline(_filename.c_str(), _errbuf);
        _empty = true;
    } // loop back and try again
}

int PcapInput::loop_until_gpstime(VeloProc *velo_proc, int loops)
{
    struct pcap_pkthdr* header;
    const u_char* pkt_data;

    int count = 0;
    while (count < loops) {
        int res = 0;
        if ((res = pcap_next_ex(_pcap, &header, &pkt_data)) >= 0) {
            if (!filter_firing_packet(header, pkt_data)) {
                continue;
            }

            velo_proc->proc_firing_data(data_buf(), pos_buf());

            if (velo_proc->got_gpstime()) {
                return Input::OK;
            }
        }
        ++count;
    }

    return Input::ERROR;
}

bool PcapInput64e::filter_firing_packet(
        const struct pcap_pkthdr* header, const u_char* pkt_data)
{
    if (header->len == _firing_pkt_size + ETHERNET_HEADER_SIZE) {
        memcpy(data_buf(), pkt_data + ETHERNET_HEADER_SIZE,
            _firing_pkt_size);
        ++_stats.rcv_cnt;
        return true;
    }
    ++_stats.sz_mismatch_cnt;
    return false;
}

bool PcapInput32eVlp16::filter_firing_packet(
    const struct pcap_pkthdr* header, const u_char* pkt_data)
{
    if (header->len == _firing_pkt_size + ETHERNET_HEADER_SIZE) {
        memcpy(data_buf(), pkt_data + ETHERNET_HEADER_SIZE,
            _firing_pkt_size);
        ++_stats.rcv_cnt;
        return true;
    }
    if (header->len == _pos_pkt_size + ETHERNET_HEADER_SIZE) {
        memcpy(pos_buf(), pkt_data + ETHERNET_HEADER_SIZE,
            _pos_pkt_size);
        ++_pos_stats.rcv_cnt;
        return true;
    }
    ++_stats.sz_mismatch_cnt;
    return false;
}

} // velodyne namespace
