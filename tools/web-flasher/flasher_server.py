#!/usr/bin/env python3
"""
Simple HTTP server for ESP32 Web Flasher with proper index routing
"""
import http.server
import socketserver
import os
from pathlib import Path
from urllib.parse import urlparse

class WebFlasherHandler(http.server.SimpleHTTPRequestHandler):
    """Custom handler that redirects root to web-flasher.html"""
    
    def end_headers(self):
        # Add CORS headers for local development
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        super().end_headers()
    
    def do_GET(self):
        parsed_path = urlparse(self.path)
        
        # Redirect root to tools/web-flasher/web-flasher.html
        if parsed_path.path == '/' or parsed_path.path == '':
            self.send_response(302)
            self.send_header('Location', '/tools/web-flasher/web-flasher.html')
            self.end_headers()
            return
        
        # Serve files normally
        return super().do_GET()
    
    def log_message(self, format, *args):
        # Custom logging - just use default format but make it prettier
        try:
            # Extract the actual request line from format string
            msg = format % args
            
            # Check what type of request this is
            if '"GET' in msg:
                # Parse: "GET /path HTTP/1.1" status -
                parts = msg.split('"')
                if len(parts) >= 2:
                    request = parts[1]  # e.g. "GET /path HTTP/1.1"
                    path = request.split()[1] if len(request.split()) > 1 else 'unknown'
                    status = parts[2].strip().split()[0] if len(parts) > 2 else 'unknown'
                    
                    if status == '200':
                        print(f"✅ {request} - 200 OK")
                    elif status == '304':
                        print(f"📄 {request} - 304 Not Modified")
                    elif status == '302':
                        print(f"↪️  {request} - 302 Redirect")
                    elif status == '404':
                        if 'favicon' not in path and 'well-known' not in path:
                            print(f"❌ {request} - 404 NOT FOUND ← THIS IS THE PROBLEM!")
                    else:
                        print(f"📝 {msg}")
                else:
                    print(f"📝 {msg}")
            else:
                print(f"📝 {msg}")
        except Exception as e:
            # Fallback to default
            print(f"📝 {format % args}")

if __name__ == '__main__':
    PORT = 8000
    
    # Change to project root directory (two levels up from web-flasher/)
    script_dir = Path(__file__).parent.resolve()  # tools/web-flasher
    tools_dir = script_dir.parent  # tools
    project_root = tools_dir.parent  # project root
    os.chdir(project_root)
    
    Handler = WebFlasherHandler
    
    # Enable SO_REUSEADDR to avoid "Address already in use" errors
    socketserver.TCPServer.allow_reuse_address = True
    
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        print(f"🚀 ESP32 Web Flasher running on port {PORT}")
        print(f"📂 Serving from: {project_root}")
        print(f"🌐 Access at: http://localhost:{PORT}/")
        print(f"   (will auto-redirect to tools/web-flasher/web-flasher.html)")
        print("")
        print("Press Ctrl+C to stop")
        print("")
        
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n👋 Server stopped")
