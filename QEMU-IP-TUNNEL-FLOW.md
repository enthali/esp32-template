# QEMU IP Tunnel Network Flow Documentation

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        HOST (Linux)                             │
│                                                                 │
│  ┌──────────┐      ┌─────────────┐      ┌─────────────────┐     │
│  │  ping    │─────▶│  tun0       │◀────▶│  TUN Bridge   │     │ 
│  │  curl    │      │ 192.168.    │      │  (Python)       │     │
│  └──────────┘      │ 100.1/24    │      └─────────────────┘     │
│                    └─────────────┘               │              │
│                                                  │ TCP:5556     │
└──────────────────────────────────────────────────┼──────────────┘
                                                   │
                                                   │ QEMU chardev
┌──────────────────────────────────────────────────┼──────────────┐
│                     ESP32 QEMU                   │              │
│                                                  ▼              │
│  ┌────────────┐      ┌──────────────┐      ┌─────────────┐      │
│  │  Web       │      │   lwIP       │      │   UART1     │      │
│  │  Server    │◀───▶│   Stack      │◀───▶│   Driver    │      │ 
│  │  HTTP      │      │ 192.168.     │      │             │      │
│  │            │      │ 100.2/24     │      └─────────────┘      │
│  └────────────┘      └──────────────┘                           │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Frame Format (Over UART)

```
┌───────────────┬────────────────────────────────────────────┐
│ Frame Length  │           Ethernet Frame                   │
│   (2 bytes)   │         (14-byte header + IP)              │
│  Big Endian   │                                            │
├───────────────┼────────────────────────────────────────────┤
│  [HI] [LO]    │ [DST_MAC:6][SRC_MAC:6][TYPE:2][IP PACKET]  │
└───────────────┴────────────────────────────────────────────┘

Example ICMP Echo Request (98 bytes total):
  Length: 0x00 0x62 (98 bytes)
  Ethernet:
    Dst MAC: 02:00:00:00:00:02 (ESP32)
    Src MAC: 02:00:00:00:00:01 (Host)
    Type:    0x08 0x00 (IPv4)
  IP:
    Version/IHL: 0x45 (IPv4, 20 byte header)
    Protocol: 0x01 (ICMP)
    Src IP: 192.168.100.1 (0xC0 0xA8 0x64 0x01)
    Dst IP: 192.168.100.2 (0xC0 0xA8 0x64 0x02)
  ICMP:
    Type: 0x08 (Echo Request)
    Code: 0x00
```

## Initialization Sequence

```
┌──────────────────────────────────────────────────────────────┐
│                  1. System Startup                           │
└──────────────────────────────────────────────────────────────┘
                           │
                           ▼
┌──────────────────────────────────────────────────────────────┐
│  2. Initialize Network Stack (wifi_manager_sim.c)            │
│     - esp_netif_init()                                       │
│     - esp_event_loop_create_default()                        │
└──────────────────────────────────────────────────────────────┘
                           │
                           ▼
┌──────────────────────────────────────────────────────────────┐
│  3. Initialize UART Tunnel Driver (netif_uart_tunnel_sim.c) │
│     a) init_uart()                                           │
│        - Configure UART1: 115200 baud, GPIO 17/16           │
│        - uart_driver_install() with 2K buffers               │
│                                                              │
│     b) Create esp_netif handle                               │
│        - ESP_NETIF_INHERENT_DEFAULT_ETH() config            │
│        - ESP_NETIF_NETSTACK_DEFAULT_ETH stack               │
│        - esp_netif_new()                                     │
│                                                              │
│     c) Configure static IP                                   │
│        - IP: 192.168.100.2                                   │
│        - Netmask: 255.255.255.255                            │
│        - Gateway: 192.168.100.1                              │
│        - esp_netif_set_ip_info()                             │
│                                                              │
│     d) Attach custom driver                                  │
│        - Allocate esp_netif_driver_base_t                    │
│        - esp_netif_attach(netif, driver)                     │
│        - Register netif_transmit() callback                  │
│        - esp_netif_set_driver_config()                       │
│                                                              │
│     e) Configure lwIP netif directly                         │
│        - Get lwip netif: esp_netif_get_netif_impl()          │
│        - Set MAC: 02:00:00:00:00:02                          │
│        - Set flags: NETIF_FLAG_ETHARP | ETHERNET | BROADCAST│
│        - netif_set_default() for routing                     │
│        - etharp_add_static_entry() for gateway               │
│                                                              │
│     f) Bring interface up                                    │
│        - esp_netif_action_connected()                        │
│                                                              │
│     g) Start RX task                                         │
│        - xTaskCreate(uart_rx_task, priority=2)               │
└──────────────────────────────────────────────────────────────┘
                           │
                           ▼
┌──────────────────────────────────────────────────────────────┐
│  4. Start TUN Bridge (Python script)                         │
│     - Create tun0 device (ioctl TUNSETIFF)                   │
│     - Configure IP: 192.168.100.1/24                         │
│     - Connect to TCP:5556 (QEMU UART1)                       │
│     - Start bidirectional bridge loop                        │
└──────────────────────────────────────────────────────────────┘
                           │
                           ▼
┌──────────────────────────────────────────────────────────────┐
│  5. System Ready                                             │
│     - esp_netif: UP, LINK_UP, flags=0x1F                     │
│     - Routes: All traffic → 192.168.100.1 gateway            │
│     - ARP: Static entry for 192.168.100.1                    │
└──────────────────────────────────────────────────────────────┘
```

## PING Packet Flow (Expected)

### Inbound Path (Host → ESP32)

```
1. Host: ping 192.168.100.2
   └─▶ Kernel creates ICMP Echo Request packet
       └─▶ Routes to tun0 (192.168.100.0/24 network)

2. TUN Bridge (Python)
   ├─▶ Reads raw IP packet from tun0
   ├─▶ Prepends Ethernet header:
   │   - Dst MAC: 02:00:00:00:00:02 (ESP32)
   │   - Src MAC: 02:00:00:00:00:01 (Host)
   │   - EtherType: 0x0800 (IPv4)
   ├─▶ Prepends 2-byte length header (big-endian)
   └─▶ Writes to TCP socket (QEMU UART1)

3. QEMU UART1 → ESP32
   └─▶ Data appears in UART RX FIFO

4. ESP32 uart_rx_task
   ├─▶ uart_read_bytes() gets length header (2 bytes)
   ├─▶ Validates frame_len (0 < len <= 1500)
   ├─▶ uart_read_bytes() gets complete frame
   └─▶ Calls esp_netif_receive(netif, buffer, len, NULL)

5. esp_netif_receive() ← *** CURRENT CRASH POINT ***
   ├─▶ Should validate netif and driver config
   ├─▶ Should allocate pbuf
   ├─▶ Should copy frame to pbuf
   └─▶ Should call tcpip_input() or ethernet_input()

6. lwIP ethernet_input() [IF REACHED]
   ├─▶ Parses Ethernet header
   ├─▶ Checks EtherType (0x0800 = IPv4)
   ├─▶ Strips Ethernet header (14 bytes)
   └─▶ Calls ip4_input() with IP packet

7. lwIP ip4_input()
   ├─▶ Validates IP header checksum
   ├─▶ Checks destination IP (192.168.100.2)
   ├─▶ Identifies protocol (ICMP = 1)
   └─▶ Calls icmp_input()

8. lwIP icmp_input()
   ├─▶ Validates ICMP checksum
   ├─▶ Identifies Echo Request (type=8)
   ├─▶ Creates Echo Reply (type=0)
   ├─▶ Swaps src/dst IPs
   └─▶ Calls ip4_output_if() to send response
```

### Outbound Path (ESP32 → Host)

```
9. lwIP ip4_output_if()
   ├─▶ Looks up route for 192.168.100.1
   ├─▶ Finds gateway 192.168.100.1 on default netif
   ├─▶ Checks ARP cache for 192.168.100.1
   ├─▶ Finds static ARP entry → MAC 02:00:00:00:00:01
   └─▶ Calls netif->linkoutput() with Ethernet frame

10. esp_netif TX Wrapper
    ├─▶ Receives Ethernet frame from lwIP
    ├─▶ Calls registered transmit callback
    └─▶ netif_transmit(driver, buffer, len)

11. netif_transmit() ← *** SHOULD SEE "TX: *** TRANSMIT CALLED ***" ***
    ├─▶ Logs TX counter increment
    ├─▶ Prepends 2-byte length header
    ├─▶ uart_write_bytes() - length header
    └─▶ uart_write_bytes() - Ethernet frame

12. ESP32 UART1 → QEMU
    └─▶ Data sent to TCP socket

13. TUN Bridge (Python)
    ├─▶ Reads from TCP socket
    ├─▶ Parses 2-byte length header
    ├─▶ Reads Ethernet frame
    ├─▶ Strips Ethernet header (14 bytes)
    ├─▶ Extracts IP packet
    └─▶ Writes IP packet to tun0

14. Host Kernel
    ├─▶ Receives ICMP Echo Reply from tun0
    ├─▶ Matches with pending ping request
    └─▶ Displays: "64 bytes from 192.168.100.2: icmp_seq=1 ttl=64 time=X ms"
```

## Current Problem Analysis

### Crash Location
```
Backtrace: esp_netif_receive at esp_netif_lwip.c:1335
           uart_rx_task at netif_uart_tunnel_sim.c:144

PC: 0x00000000 (InstrFetchProhibited)
└─▶ strlen called on NULL pointer inside esp_netif_receive
```

### Root Cause Hypothesis

The `esp_netif_receive()` function expects:
1. ✅ Valid `esp_netif_t*` handle (we have this)
2. ✅ Valid buffer pointer (we allocate this)
3. ✅ Valid length (we validate this)
4. ❌ **Properly initialized esp_netif driver structure**

The crash at `strlen(NULL)` suggests esp_netif is trying to access driver metadata that wasn't properly set up.

### What's Missing

Looking at ESP-IDF source, `esp_netif_receive()` internally:
```c
esp_err_t esp_netif_receive(esp_netif_t *esp_netif, void *buffer, size_t len, void *eb)
{
    // ... validation ...
    
    // Gets driver handle from esp_netif
    esp_netif_driver_ifconfig_t *driver = esp_netif->driver_handle;
    
    // May call driver->driver_free_rx_buffer(driver->handle, eb)
    // ^^^ This dereferences driver->handle which might be NULL!
    
    // Then forwards to lwIP...
}
```

### The Fix

We have two options:

**Option A: Bypass esp_netif_receive entirely**
- Call lwIP `ethernet_input()` directly (we tried this, got different crash)
- Need to properly set `netif->linkoutput` function pointer
- More low-level but gives us full control

**Option B: Fix esp_netif driver setup**
- Ensure `driver_handle` in esp_netif is properly initialized
- May need to call `esp_netif_attach()` with a complete driver structure
- The driver structure needs specific fields populated

## Recommended Next Steps

1. **Verify driver attachment**:
   - Check if `esp_netif_attach()` succeeded
   - Verify `esp_netif_get_driver_ifconfig()` returns valid data
   - Add logging to confirm driver callbacks are registered

2. **Alternative: Direct lwIP integration**:
   - Set `netif->linkoutput = netif_linkoutput_func` directly
   - Set `netif->output = etharp_output` for ARP handling
   - Call `ethernet_input()` for RX instead of `esp_netif_receive()`
   - This bypasses esp_netif abstraction layer entirely

3. **Debugging aids**:
   - Add `ESP_LOGI` before `esp_netif_receive()` call
   - Log `esp_netif_get_driver_ifconfig()` result
   - Check if `s_netif_handle` is still valid at RX time
   - Verify driver base pointer hasn't been freed

## Reference: Working ESP-IDF Ethernet Driver Pattern

```c
// Typical Ethernet driver initialization in ESP-IDF
esp_err_t driver_init(void) {
    // 1. Create driver handle
    driver_t *driver = calloc(1, sizeof(driver_t));
    
    // 2. Create base structure
    esp_netif_driver_base_t *base = &driver->base;
    base->post_attach = driver_post_attach_callback;
    
    // 3. Create esp_netif
    esp_netif_t *netif = esp_netif_new(&cfg);
    
    // 4. Attach driver
    esp_netif_attach(netif, base);
    
    // 5. Set driver config
    esp_netif_driver_ifconfig_t ifconfig = {
        .handle = driver,
        .transmit = driver_transmit,
        .driver_free_rx_buffer = driver_free_buffer
    };
    esp_netif_set_driver_config(netif, &ifconfig);
    
    // 6. Get lwIP netif and set linkoutput
    struct netif *lwip_netif = esp_netif_get_netif_impl(netif);
    lwip_netif->linkoutput = low_level_output;
    
    // 7. Bring up
    esp_netif_action_connected(netif, 0, 0, NULL);
}

// RX path
void rx_task(void) {
    uint8_t *buffer = malloc(MAX_SIZE);
    size_t len = read_packet(buffer);
    
    // Option 1: Use esp_netif (higher level)
    esp_netif_receive(netif, buffer, len, buffer); // eb=buffer for free
    
    // Option 2: Direct lwIP (lower level)
    struct pbuf *p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    memcpy(p->payload, buffer, len);
    ethernet_input(p, lwip_netif);
    free(buffer);
}

// TX path
static err_t low_level_output(struct netif *netif, struct pbuf *p) {
    // Called by lwIP when sending
    // p->payload points to Ethernet frame
    // p->tot_len is total length
    send_to_uart(p->payload, p->tot_len);
    return ERR_OK;
}
```

## MAC Addresses

- **Host (TUN Bridge)**: `02:00:00:00:00:01`
- **ESP32 (UART Tunnel)**: `02:00:00:00:00:02`

Both are locally administered unicast MACs (bit 1 of first octet set).

## IP Configuration

- **Host**: 192.168.100.1/24 (gateway)
- **ESP32**: 192.168.100.2/24
- **Gateway**: 192.168.100.1
- **Netmask**: 255.255.255.0

## Port Assignments

- **QEMU UART0**: TCP port 5555 (console/monitor)
- **QEMU UART1**: TCP port 5556 (IP tunnel)

---

*Last Updated: 2025-09-30*
*Status: RX path crashes in esp_netif_receive(), TX path never reached*
