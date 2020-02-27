/** Copyright (C) 2016 - 2020 European Spallation Source ERIC */
#define __STDC_FORMAT_MACROS 1

#include <CLI/CLI.hpp>
#include <cinttypes>
#include <iostream>
#include <common/Socket.h>
#include <common/TSCTimer.h>
#include <common/Timer.h>
#include <stdio.h>
#include <unistd.h>

#define TSC_MHZ 3000

struct {
  std::string IpAddress{"127.0.0.1"};
  int UDPPort{9000};
  int DataSize{9000};
  int SocketBufferSize{4000000};
} Settings;

CLI::App app{"UDP transmitter with 32 bit sequence number."};

char Buffer[10000];

int main(int argc, char *argv[]) {
  app.add_option("-p, --port", Settings.UDPPort, "UDP transmit port");
  app.add_option("-d, --data_size", Settings.DataSize, "Size of UDP payload (bytes)");
  app.add_option("-b, --socket_buffer_size", Settings.SocketBufferSize, "socket buffer size (bytes)");
  CLI11_PARSE(app, argc, argv);

  uint64_t TxBytesTotal{0};
  uint64_t TxBytes{0};
  uint64_t TxPackets{0};
  const int B1M{1000000};

  Socket::Endpoint Local("0.0.0.0", 0);
  Socket::Endpoint Remote(Settings.IpAddress.c_str(), Settings.UDPPort);
  UDPTransmitter UDPTx(Local, Remote);
  UDPTx.setBufferSizes(Settings.SocketBufferSize, Settings.SocketBufferSize);
  UDPTx.printBufferSizes();

  Timer RateTimer;
  TSCTimer ReportTimer;
  uint32_t SeqNo{1};

  for (;;) {
    *((uint32_t *)Buffer) = SeqNo;
    auto TxTmpBytes = UDPTx.send(Buffer, Settings.DataSize);

    if (TxTmpBytes > 0) {
      SeqNo++;
      TxPackets++;
      TxBytes += TxTmpBytes;
    }

    if (ReportTimer.timetsc() >= 1000000UL * TSC_MHZ) {
      auto USecs = RateTimer.timeus();
      TxBytesTotal += TxBytes;
      printf("Tx rate: %f Mbps, %f pps, tx %llu MB (total: %llu MB) %llu usecs\n",
             TxBytes * 8.0 / USecs, TxPackets * 1000000.0 / USecs, TxBytes / B1M,
             TxBytesTotal / B1M, USecs);
      TxBytes = 0;
      TxPackets = 0;
      RateTimer.now();
      ReportTimer.now();
    }
  }
}
