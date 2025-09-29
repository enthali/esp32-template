# Serial HTTP Tunneling - Planning Document

## Overview

**Goal**: Enable webserver access in QEMU emulator by tunneling HTTP traffic over the serial connection.

**Problem**: QEMU ESP32 emulation doesn't support network devices (`open_eth` not available), so the webserver can initialize but cannot receive/send network packets.

**Solution**: Multiplex HTTP traffic and log output over the serial connection using a custom framing protocol.

## Architecture

```
Browser → localhost:8080 → Python Proxy → Serial (multiplexed) → QEMU → ESP32 App
                              ↓                                         ↓
                        Demux: logs → stdout                    Demux: HTTP → webserver
                               HTTP → socket                           logs → UART
```

## Implementation Phases

### Phase 1: Serial Pass-through Proxy (Proof of Concept)
**Goal**: Verify we can intercept and re-output QEMU serial without breaking existing functionality.

- [ ] **Task 1.1**: Modify `run_qemu_with_webserver.sh` to redirect QEMU serial to unix socket
  - Change `-serial stdio` to `-serial unix:/tmp/esp32-serial.sock,server,nowait`
  - Start QEMU in background
  - Test that QEMU starts successfully and creates the socket

- [ ] **Task 1.2**: Create basic Python proxy (`tools/serial_proxy.py`)
  - Connect to unix socket `/tmp/esp32-serial.sock`
  - Read raw data from socket
  - Write raw data to stdout (pass-through mode)
  - Handle connection errors gracefully

- [ ] **Task 1.3**: Test pass-through proxy
  - Run modified script
  - Verify all logs appear in terminal exactly as before
  - Verify interactive features still work (if any)
  - Test Ctrl+C behavior and cleanup

- [ ] **Task 1.4**: Add bidirectional support (if needed)
  - Read from stdin (non-blocking)
  - Forward stdin to QEMU serial socket
  - Test interactive commands (if applicable)

**Exit Criteria**: QEMU runs with serial through proxy, all logs visible, no functional differences from direct serial.

---

### Phase 2: Framing Protocol Design
**Goal**: Design and document the multiplexing protocol.

- [ ] **Task 2.1**: Define frame format
  ```
  Frame Structure:
  [ESCAPE_SEQ][TYPE][LENGTH_MSB][LENGTH_LSB][DATA...][CHECKSUM]
  
  - ESCAPE_SEQ: 0xAA 0x55 (2 bytes, distinctive pattern)
  - TYPE: 0x01 = LOG, 0x02 = HTTP_REQUEST, 0x03 = HTTP_RESPONSE
  - LENGTH: 16-bit big-endian (max 65535 bytes per frame)
  - DATA: variable length payload
  - CHECKSUM: simple 8-bit XOR of all bytes (or CRC-8)
  ```

- [ ] **Task 2.2**: Handle escape sequence in payload
  - If data contains 0xAA 0x55, use byte stuffing:
    - Replace 0xAA with 0xAA 0x00
    - Decoder: 0xAA 0x00 → 0xAA
  - Alternative: Use COBS (Consistent Overhead Byte Stuffing)

- [ ] **Task 2.3**: Design error handling
  - Frame timeout (incomplete frame)
  - Checksum mismatch
  - Invalid type byte
  - Buffer overflow protection

- [ ] **Task 2.4**: Document protocol
  - Create `docs/design/serial-tunneling-protocol.md`
  - Include frame diagrams
  - Document error handling
  - Add example frames

**Exit Criteria**: Protocol fully specified and documented.

---

### Phase 3: Python Proxy with Multiplexing
**Goal**: Implement frame parsing and demultiplexing in Python proxy.

- [ ] **Task 3.1**: Implement frame parser
  - Create `tools/serial_mux_proxy.py`
  - Implement frame detection (escape sequence scanner)
  - Parse frame header (type, length)
  - Validate checksum
  - Extract payload

- [ ] **Task 3.2**: Implement frame encoder
  - Function to create frames from data
  - Apply byte stuffing if needed
  - Calculate checksum
  - Serialize frame

- [ ] **Task 3.3**: Add demultiplexing logic
  - Route FRAME_LOG to stdout
  - Buffer FRAME_HTTP for later processing
  - Handle partial frames (spanning multiple reads)
  - Log parsing errors

- [ ] **Task 3.4**: Test with mock data
  - Create test script that generates framed data
  - Feed to proxy via pipe or socket
  - Verify correct demultiplexing
  - Test error handling (corrupted frames)

**Exit Criteria**: Proxy can correctly parse and demultiplex framed data.

---

### Phase 4: ESP32 Serial Output Framing
**Goal**: Wrap ESP32 log output in frames.

- [ ] **Task 4.1**: Create serial framing component
  - New component: `components/serial_tunnel/`
  - Header: `serial_tunnel.h`
  - Implementation: `serial_tunnel.c`
  - CMakeLists.txt

- [ ] **Task 4.2**: Implement frame output functions
  ```c
  void serial_tunnel_send_log(const char* data, size_t len);
  void serial_tunnel_send_http(const uint8_t* data, size_t len);
  ```
  - Create frame with proper header
  - Apply byte stuffing
  - Write to UART

- [ ] **Task 4.3**: Hook into ESP-IDF logging
  - Create custom vprintf handler
  - Intercept all log output
  - Wrap in FRAME_LOG
  - Send via serial_tunnel
  - Consider log level filtering

- [ ] **Task 4.4**: Test ESP32 side
  - Build with new component
  - Run in QEMU with multiplexing proxy
  - Verify logs still appear correctly
  - Check frame format with debug output
  - Test high-volume logging (stress test)

**Exit Criteria**: All ESP32 logs wrapped in frames, visible through proxy.

---

### Phase 5: HTTP Server Serial Backend
**Goal**: Route HTTP traffic through serial instead of network stack.

- [ ] **Task 5.1**: Research ESP-IDF HTTP server internals
  - Understand `esp_http_server` socket handling
  - Find hook points for custom transport
  - Check if we can replace lwIP sockets with custom I/O

- [ ] **Task 5.2**: Design HTTP-over-serial adapter
  - Option A: Modify `esp_http_server` to use custom transport
  - Option B: Create wrapper that translates serial ↔ HTTP
  - Option C: Use existing abstraction (e.g., `esp_transport`)
  - Document chosen approach

- [ ] **Task 5.3**: Implement serial HTTP transport
  ```c
  // Redirect HTTP server socket operations to serial frames
  int serial_http_read(void* ctx, uint8_t* buf, size_t len);
  int serial_http_write(void* ctx, const uint8_t* buf, size_t len);
  ```
  - Receive FRAME_HTTP_REQUEST from serial
  - Pass to HTTP server
  - Send response via FRAME_HTTP_RESPONSE

- [ ] **Task 5.4**: Handle multiple HTTP connections
  - Add connection ID to frame header (TYPE byte extension?)
  - Maintain connection state table
  - Route frames to correct connection

- [ ] **Task 5.5**: Conditional compilation
  - Use `#ifdef CONFIG_SERIAL_HTTP_TUNNEL`
  - Keep normal network path for real hardware
  - Minimal code changes to main application

**Exit Criteria**: HTTP server can receive requests and send responses via serial.

---

### Phase 6: Python Proxy HTTP Server
**Goal**: Accept HTTP connections on localhost:8080 and forward via serial.

- [ ] **Task 6.1**: Add HTTP server to proxy
  - Listen on `localhost:8080`
  - Accept incoming connections
  - Read HTTP requests

- [ ] **Task 6.2**: Implement HTTP ↔ serial bridging
  - Receive HTTP request from browser
  - Wrap in FRAME_HTTP_REQUEST
  - Send to ESP32 via serial
  - Wait for FRAME_HTTP_RESPONSE
  - Send response back to browser

- [ ] **Task 6.3**: Handle connection multiplexing
  - Assign connection IDs
  - Track active connections
  - Route responses to correct browser connection
  - Handle connection timeouts

- [ ] **Task 6.4**: Optimize for serial bandwidth
  - Compress responses if possible
  - Strip unnecessary HTTP headers
  - Consider HTTP/1.0 vs HTTP/1.1 (persistent connections)

**Exit Criteria**: Browser can access `http://localhost:8080` and receive responses from ESP32.

---

### Phase 7: Integration and Testing
**Goal**: End-to-end system testing and polish.

- [ ] **Task 7.1**: Integration testing
  - Build complete system
  - Test all web endpoints: `/`, `/config`, `/api/*`
  - Test static resources (CSS, JS)
  - Verify JSON API responses

- [ ] **Task 7.2**: Performance testing
  - Measure latency (HTTP request → response)
  - Test with concurrent requests
  - Monitor serial bandwidth usage
  - Optimize if needed (buffer sizes, frame size)

- [ ] **Task 7.3**: Error handling and recovery
  - Test with connection drops
  - Test with malformed HTTP requests
  - Test with frame corruption
  - Verify graceful degradation

- [ ] **Task 7.4**: User experience polish
  - Add status messages to proxy (connection established, etc.)
  - Color-code log output (if terminal supports)
  - Add proxy command-line options (--port, --debug, etc.)
  - Improve shutdown handling (Ctrl+C cleanup)

- [ ] **Task 7.5**: Documentation
  - Update `docs/emulator-webserver-access.md`
  - Add architecture diagram
  - Document limitations and known issues
  - Add troubleshooting section

- [ ] **Task 7.6**: Update build scripts
  - Integrate into existing workflow
  - Add `idf.py qemu-web` command (if possible)
  - Update README with new instructions

**Exit Criteria**: Fully working webserver access in QEMU, documented and polished.

---

## Technical Decisions to Make

1. **Frame size limit**: 64KB (uint16 length) vs smaller (e.g., 4KB chunks)
2. **Checksum algorithm**: Simple XOR vs CRC-8 vs CRC-16
3. **HTTP server modification approach**: Custom transport vs wrapper vs fork
4. **Serial baud rate**: Stay at 115200 or increase to 921600? (QEMU may not care)
5. **Threading model**: Python asyncio vs threading vs multiprocessing
6. **Connection ID encoding**: Extend TYPE byte vs separate field in frame

## Known Limitations

- **Serial bandwidth**: 115200 baud ≈ 11 KB/s (sufficient for HTML/JSON, not for images)
- **Latency**: Serial adds overhead compared to native networking
- **Debugging**: Harder to see raw HTTP traffic (wrapped in frames)
- **No real network stack testing**: This bypasses lwIP entirely

## Future Enhancements

- [ ] Support for WebSocket tunneling (real-time data streaming)
- [ ] Compression (gzip responses before framing)
- [ ] HTTPS support (tunnel TLS over serial)
- [ ] Port to other QEMU platforms (ESP32-S3, ESP32-C3)
- [ ] Upsteam contribution to Espressif QEMU (if valuable)

## References

- **SLIP Protocol**: [RFC 1055](https://datatracker.ietf.org/doc/html/rfc1055)
- **PPP Protocol**: [RFC 1661](https://datatracker.ietf.org/doc/html/rfc1661)
- **COBS Encoding**: [Wikipedia](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing)
- **ESP-IDF HTTP Server**: [docs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/esp_http_server.html)
- **ESP-IDF Custom Transport**: [example](https://github.com/espressif/esp-idf/tree/master/examples/protocols/esp_http_client)

## Branch Strategy

- `feature/enable-web-in-emulator` - Current branch (webserver works but not accessible)
- Create `feature/serial-http-tunnel` - For this work
- Keep phases in separate commits for easy rollback

## Time Estimates

- Phase 1: 2-4 hours
- Phase 2: 2-3 hours
- Phase 3: 4-6 hours
- Phase 4: 4-6 hours
- Phase 5: 8-12 hours (most complex)
- Phase 6: 4-6 hours
- Phase 7: 4-8 hours

**Total**: ~30-45 hours of development

---

## Current Status

✅ **Phase 0: Research and planning (COMPLETE)**
- Confirmed QEMU lacks network device emulation (`open_eth` not available on ESP32 or ESP32-S3)
- Verified serial is only I/O channel
- Designed serial tunneling architecture
- Created this planning document
- **Already completed**: Fixed `wifi_manager_sim.c` to initialize network stack and start webserver
  - The webserver successfully initializes in QEMU (see logs)
  - All 19 URI handlers registered
  - But no network device to handle actual HTTP traffic

⏸️ **Paused** - Ready to resume with Phase 1 when you return!

## What We Already Fixed

The `wifi_manager_sim.c` already has these working changes:
- ✅ Network interface layer initialization (`esp_netif_init()`)
- ✅ Event loop initialization (`esp_event_loop_create_default()`)
- ✅ Webserver initialization and startup
- ✅ All URI handlers registered

These changes are preserved and will work with the serial tunneling solution.

---

**Last Updated**: September 29, 2025
**Status**: Planning complete, ready for implementation
**Branch**: `feature/enable-web-in-emulator` (will create `feature/serial-http-tunnel` for implementation)
