/*********************************************************************** *
 *
 * Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
 *
 ************************************************************************/

/*
 * @file   input.h
 * @author fengkaiwen01@ (original), feiaiguo@ (re-design/implementation)
 *         youxiangtao@ (May 2018)
 * @brief
 */

#ifndef VELODYNE_DRIVER_PCAP_INPUT_H
#define VELODYNE_DRIVER_PCAP_INPUT_H

#include <unistd.h>
#include <string.h>
#include <pcap.h>

#include "input.h"
#include "rate.h"

namespace velodyne_driver {

/** @brief Velodyne input from PCAP dump file.
 *
 * Dump files can be grabbed by libpcap, Velodyne's DSR software,
 * ethereal, wireshark, tcpdump, or the vdump_command.
 */
class PcapInput: public Input {
public:
    /** @brief constructor
     *
     *  @param packet_rate expected device packet frequency (Hz)
     *  @param filename PCAP dump file name
     *  @param read_once true to read pcap file only once,
     *   false to read pcap in a loop
     *  @param read_fast true to read pcap file as fast as possible,
     *   false read pcap file at the speed of the given packet_rate
     *  @param repeat_delay time to wait before repeating PCAP data
     */
    PcapInput(
            double packet_rate,
            const std::string &filename,
            uint16_t port,
            uint16_t pos_port,
			int firing_pkt_size,
			int pos_pkt_size,
            bool read_once = false,
            bool read_fast = false,
            double repeat_delay = 0.0,
            bool live = true) :
        Input(),
        _packet_rate(packet_rate, 10),
        _filename(filename),
        _port(port),
        _pos_port(pos_port),
		_firing_pkt_size(firing_pkt_size),
		_pos_pkt_size(pos_pkt_size),
        _read_once(read_once),
        _read_fast(read_fast),
        _repeat_delay(repeat_delay),
        _live(live)
    {
        _pcap = NULL;
        _empty = true;
        _data.reset(new uint8_t[_firing_pkt_size]);
        _pos.reset(new uint8_t[_pos_pkt_size]);
        memset(_errbuf, 0, sizeof(_errbuf));
    }

    virtual ~PcapInput();

    virtual bool init();

    virtual int get_firing_data_packet(bool block);

    virtual int loop_until_gpstime(VeloProc *velo_proc, int loops);

    virtual uint8_t *data_buf()
    {
        return _data.get();
    }

    virtual uint8_t *pos_buf()
    {
        return _pos.get();
    }

    virtual void log_stats()
    {
        VELO_LOG_INFO("    port %d: ", _port);
        _stats.log();

	if (_pos_port != _port) {
            VELO_LOG_INFO("    port %d: ", _pos_port);
            _pos_stats.log();
	}
    }

protected:
    /// @brief returns true if the given packet is a valid firing data packet that
    /// should be processed.
    virtual bool filter_firing_packet(
            const struct pcap_pkthdr* header, const u_char* pkt_data) = 0;

    // Used to control the speed of reading from pcap file.
    Rate _packet_rate;
    std::string _filename;
    uint16_t _port;
    uint16_t _pos_port;
	int _firing_pkt_size;
	int _pos_pkt_size;
    bool _read_once;
    bool _read_fast;
    double _repeat_delay;
    bool _live;

    pcap* _pcap;
    bool _empty;
    std::unique_ptr<uint8_t[]> _data;
    std::unique_ptr<uint8_t[]> _pos;
    char _errbuf[PCAP_ERRBUF_SIZE];
    Input::Stats _stats;
    Input::Stats _pos_stats;

    DISALLOW_COPY_AND_ASSIGN(PcapInput);
};

/*
 * @brief PCAP input for Velodyne 64E
 */
class PcapInput64e: public PcapInput {
public:
    PcapInput64e(
            double packet_rate,
            const std::string &filename,
            uint16_t port,
			int firing_pkt_size,
			int pos_pkt_size,
            bool live = true)
        : PcapInput(packet_rate, filename, port, port, firing_pkt_size, pos_pkt_size,
            false, false, 0.0, live) {}

    uint8_t *pos_buf()
    {
        return _data.get();
    }

protected:
    bool filter_firing_packet(
            const struct pcap_pkthdr* header, const u_char* pkt_data);

    DISALLOW_COPY_AND_ASSIGN(PcapInput64e);
};

/*
 * @brief PCAP input, for Velodyne 32E and VLP16 with positioning data
 *  processing.
 */
class PcapInput32eVlp16: public PcapInput {
public:
    PcapInput32eVlp16(
            double packet_rate,
            const std::string &filename,
            uint16_t port,
            uint16_t pos_port,
			int firing_pkt_size,
			int pos_pkt_size,
            bool live = true)
        : PcapInput(packet_rate, filename, port, pos_port, firing_pkt_size, pos_pkt_size,
            false, false, 0.0, live) {}

protected:
    bool filter_firing_packet(
            const struct pcap_pkthdr* header, const u_char* pkt_data);

    DISALLOW_COPY_AND_ASSIGN(PcapInput32eVlp16);
};

} // velodyne_driver namespace

#endif // VELODYNE_DRIVER_PCAP_INPUT_H
