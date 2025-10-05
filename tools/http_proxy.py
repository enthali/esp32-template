#!/usr/bin/env python3
"""
Simple HTTP proxy to forward localhost:8080 -> 192.168.100.2:80
This allows VS Code Simple Browser to access the ESP32 webserver.
"""
import http.server
import urllib.request
import socketserver

PORT = 8080
ESP32_URL = "http://192.168.100.2"

class ProxyHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        target_url = ESP32_URL + self.path
        print(f"Proxying GET: {self.path} -> {target_url}")
        try:
            with urllib.request.urlopen(target_url, timeout=10) as response:
                self.send_response(response.status)
                for header, value in response.headers.items():
                    self.send_header(header, value)
                self.end_headers()
                self.wfile.write(response.read())
        except Exception as e:
            print(f"Proxy error: {e}")
            self.send_error(502, f"Proxy Error: {e}")
    
    def log_message(self, format, *args):
        print(f"{self.address_string()} - {format % args}")

if __name__ == "__main__":
    with socketserver.TCPServer(("", PORT), ProxyHandler) as httpd:
        print(f"HTTP Proxy running on http://localhost:{PORT}")
        print(f"Forwarding to ESP32 at {ESP32_URL}")
        print("Press Ctrl+C to stop")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nProxy stopped")
