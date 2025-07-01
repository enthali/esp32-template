# ESP32 Distance Project - Current Development Roadmap

This document contains the immediate next steps for the ESP32 Distance Project. Once these are completed, we'll pick the next items from `Features-intended.md`.

---

## 4 HTTPS Security Implementation 📋 **NEXT**

### Step 4.1: Certificate Generation and Embedding ✅ **COMPLETED**
- ✅ **Build-time Certificate Generation**: Automated self-signed certificate creation during ESP-IDF build
- ✅ **Certificate Embedding**: Embed certificates as binary data in firmware
- ✅ **Certificate Validation**: 25-year validity period for long device lifecycle
- ✅ **OpenSSL Integration**: Use OpenSSL tools in build process for certificate generation

**Implementation Strategy:**
- Add CMake build script to generate certificates if they don't exist
- Embed certificates using ESP-IDF's `EMBED_FILES` feature
- Create `components/certificates/` with build-time certificate generation
- Use self-signed certificates (perfect for local IoT devices)
- **Certificate Tool Options**: OpenSSL binary OR Python cryptography library

**Build Integration:**
```cmake
# In components/certificates/CMakeLists.txt
set(COMPONENT_EMBED_FILES
    "server_cert.pem"
    "server_key.pem"
)

# Pre-build certificate generation (Option 1: OpenSSL)
add_custom_command(
    OUTPUT server_cert.pem server_key.pem
    COMMAND openssl req -x509 -newkey rsa:2048 -keyout server_key.pem -out server_cert.pem -days 9125 -nodes -subj "/CN=ESP32-Distance-Sensor"
    COMMENT "Generating self-signed certificate for HTTPS (25-year validity)"
)

# Alternative: Python-based generation (Option 2: No OpenSSL required)
# add_custom_command(
#     OUTPUT server_cert.pem server_key.pem
#     COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../../tools/generate_cert.py
#     COMMENT "Generating self-signed certificate using Python"
# )
```

**Tool Requirements:**
 Python cryptography library (included in ESP-IDF Python environment)

**Build Integration:**
```cmake
# In components/certificates/CMakeLists.txt
set(COMPONENT_EMBED_FILES
    "server_cert.pem"
    "server_key.pem"
)

# Pre-build certificate generation
add_custom_command(
    OUTPUT server_cert.pem server_key.pem
    COMMAND openssl req -x509 -newkey rsa:2048 -keyout server_key.pem -out server_cert.pem -days 9125 -nodes -subj \"/CN=ESP32-Distance-Sensor\"
    COMMENT \"Generating self-signed certificate for HTTPS (25-year validity)\"
)
```

**Deliverables:**
- Automated certificate generation integrated into build system
- Self-signed certificates embedded in firmware
- 25-year certificate validity for long device lifecycle
- No manual certificate management required
- **Two implementation options**: OpenSSL binary OR Python-based generation
- **`tools/generate_cert.py`**: Fallback certificate generator (no OpenSSL dependency)

---

### Step 4.2: HTTPS Server Implementation 📋 **NEXT** 
- 📋 **Replace HTTP Server**: Migrate from `esp_http_server` to `esp_https_server`
- 📋 **Port 443 Configuration**: Configure HTTPS server on standard port 443
- 📋 **SSL/TLS Configuration**: Proper SSL configuration with embedded certificates
- 📋 **Memory Optimization**: Optimize SSL buffer sizes for ESP32 constraints

**HTTPS Server Configuration:**
```c
#include \"esp_https_server.h\"
#include \"esp_tls.h\"

// Certificate references (embedded at build time)
extern const uint8_t server_cert_pem_start[] asm(\"_binary_server_cert_pem_start\");
extern const uint8_t server_cert_pem_end[]   asm(\"_binary_server_cert_pem_end\");
extern const uint8_t server_key_pem_start[]  asm(\"_binary_server_key_pem_start\");
extern const uint8_t server_key_pem_end[]    asm(\"_binary_server_key_pem_end\");

// HTTPS server configuration
httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();
conf.httpd.server_port = 443;
conf.httpd.max_open_sockets = 4;  // ESP32 memory optimization
conf.servercert = server_cert_pem_start;
conf.servercert_len = server_cert_pem_end - server_cert_pem_start;
conf.prvtkey_pem = server_key_pem_start;
conf.prvtkey_len = server_key_pem_end - server_key_pem_start;
```

**Implementation Strategy:**
- Replace all HTTP server calls with HTTPS equivalents
- Update WiFi manager to start HTTPS server instead of HTTP
- Configure SSL buffer sizes for ESP32 memory constraints
- Test HTTPS functionality with self-signed certificates

**Deliverables:**
- Fully functional HTTPS server on port 443
- All web interface functionality working over HTTPS
- Encrypted transmission of WiFi credentials and sensor data
- Proper SSL/TLS configuration optimized for ESP32

---

### Step 4.3: HTTP Redirect Server (User Experience) 📋
- 📋 **Lightweight HTTP Server**: Minimal HTTP server on port 80 for redirects only
- 📋 **301 Permanent Redirects**: Redirect all HTTP requests to HTTPS equivalent
- 📋 **Memory Efficient**: Minimal memory footprint for redirect functionality
- 📋 **User-Friendly URLs**: Handle both `http://192.168.4.1/` and direct HTTPS access

**Redirect Strategy:**
```c
// Lightweight HTTP redirect handler
esp_err_t http_redirect_handler(httpd_req_t *req) {
    char redirect_url[128];
    snprintf(redirect_url, sizeof(redirect_url), \"https://%s%s\", 
             get_device_ip(), req->uri);
    
    httpd_resp_set_status(req, \"301 Moved Permanently\");
    httpd_resp_set_hdr(req, \"Location\", redirect_url);
    httpd_resp_send(req, \"<html><body><a href=\\\"\", strlen(\"<html><body><a href=\\\"\"));
    httpd_resp_send(req, redirect_url, strlen(redirect_url));
    httpd_resp_send(req, \"\\\">Click here for HTTPS</a></body></html>\", 
                   strlen(\"\\\">Click here for HTTPS</a></body></html>\"));
    return ESP_OK;
}
```

**Benefits:**
- **User Experience**: Typing `http://192.168.4.1/` automatically works
- **Security**: All actual functionality happens over HTTPS
- **Memory Efficient**: HTTP server only handles redirects (minimal resources)
- **Standards Compliant**: Proper HTTP 301 redirect responses

**Deliverables:**
- Lightweight HTTP redirect server on port 80
- Automatic redirection to HTTPS for all HTTP requests
- User-friendly experience for accessing device web interface
- Minimal memory overhead for redirect functionality

---

### Step 4.4: Security Hardening and Testing 📋
- 📋 **Remove HTTP Functionality**: Ensure no sensitive data is transmitted over HTTP
- 📋 **Browser Testing**: Verify certificate acceptance workflow across browsers
- 📋 **Security Headers**: Add appropriate HTTPS security headers
- 📋 **Documentation**: User guide for accepting self-signed certificates

**Security Enhancements:**
```c
// Add security headers to HTTPS responses
httpd_resp_set_hdr(req, \"Strict-Transport-Security\", \"max-age=31536000\");
httpd_resp_set_hdr(req, \"X-Content-Type-Options\", \"nosniff\");
httpd_resp_set_hdr(req, \"X-Frame-Options\", \"DENY\");
```

**Testing Strategy:**
- Test certificate acceptance on Chrome, Firefox, Safari, Edge
- Verify all WiFi credential transmission is encrypted
- Confirm captive portal works with HTTPS
- Test both AP mode and STA mode HTTPS functionality

**Documentation Deliverables:**
- User guide for accepting self-signed certificates
- Browser-specific instructions for certificate warnings
- Security benefits explanation for end users
- Troubleshooting guide for HTTPS connection issues

**Deliverables:**
- Complete HTTPS implementation with security hardening
- Comprehensive browser compatibility testing
- User documentation for certificate acceptance
- Secure transmission of all sensitive data

---

## Architecture Benefits

After HTTPS implementation:
- **Encrypted Communication**: All web traffic protected by SSL/TLS
- **Secure Credential Transmission**: WiFi passwords encrypted in transit
- **Industry Standard Security**: Following IoT security best practices
- **User-Friendly**: Automatic HTTP→HTTPS redirection
- **Certificate Management**: Automated, no manual certificate handling

## Next Steps After HTTPS

Once HTTPS is complete, we'll have a secure foundation for:
- Configuration management system
- Real-time data streaming
- JSON API endpoints
- Advanced web interface features