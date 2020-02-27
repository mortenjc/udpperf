/** Copyright (C) 2016 - 2020 European Spallation Source ERIC */
#define __STDC_FORMAT_MACROS 1

#include <CLI/CLI.hpp>
#include <cassert>
#include <inttypes.h>
#include <iostream>
#include <common/Socket.h>
#include <common/Timer.h>
#include <stdio.h>

struct {
  int UDPPort{9000};
  int DataSize{9000};
  int SocketBufferSize{2000000};
} Settings;

CLI::App app{"UDP receiver with 32 bit sequence number check."};

int main(int argc, char *argv[]) {
  app.add_option("-p, --port", Settings.UDPPort, "UDP receive port");
  app.add_option("-b, --socket_buffer_size", Settings.SocketBufferSize, "socket buffer size (bytes)");
  CLI11_PARSE(app, argc, argv);

  static const int BUFFERSIZE{9200};
  char buffer[BUFFERSIZE];
  uint64_t RxBytesTotal{0};
  uint64_t RxBytes{0};
  uint64_t RxPackets{0};
  uint32_t SeqNo{0};
  const int intervalUs = 1000000;
  const int B1M = 1000000;

  Socket::Endpoint local("0.0.0.0", Settings.UDPPort);
  UDPReceiver Receive(local);
  Receive.setBufferSizes(Settings.SocketBufferSize, Settings.SocketBufferSize);
  Receive.printBufferSizes();

  Timer UpdateTimer;
  auto USecs = UpdateTimer.timeus();

  for (;;) {
    int ReadSize = Receive.receive(buffer, BUFFERSIZE);

    assert(ReadSize > 0);
    assert(ReadSize == Settings.DataSize);

    if (ReadSize > 0) {
      SeqNo = *((uint32_t *)buffer);
      RxBytes += ReadSize;
      RxPackets++;
    }

    assert(RxPackets <= SeqNo);

    if ((RxPackets % 100) == 0)
      USecs = UpdateTimer.timeus();

    if (USecs >= intervalUs) {
      RxBytesTotal += RxBytes;
      printf("Rx rate: %.2f Mbps, rx %" PRIu64 " MB (total: %" PRIu64
             " MB) %" PRIu64 " usecs, PER %4.3e\n",
             RxBytes * 8.0 / (USecs / 1000000.0) / B1M, RxBytes / B1M, RxBytesTotal / B1M,
             USecs, (1.0 * SeqNo - RxPackets)/SeqNo);
      RxBytes = 0;
      UpdateTimer.now();
      USecs = UpdateTimer.timeus();
    }
  }
}
