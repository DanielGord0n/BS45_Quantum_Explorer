import os
import re
import json
import subprocess
import time
from http.server import SimpleHTTPRequestHandler
from socketserver import TCPServer

PORT = 8080
DIRECTORY = "public"

class CustomHandler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def do_GET(self):
        if self.path == '/':
            self.path = '/index.html'
            return super().do_GET()
        elif self.path == '/api/status':
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            # Check daemon status
            try:
                out = subprocess.run(['./manage_solver.sh', 'status'], capture_output=True, text=True).stdout
                if 'RUNNING' in out:
                    state = 'RUNNING'
                elif 'PAUSED' in out:
                    state = 'PAUSED'
                else:
                    state = 'STOPPED'
            except Exception:
                state = 'STOPPED'
                
            # Parse log file for epochs/time
            epochs = 0
            time_el = 0.0
            
            if os.path.exists('solver.log'):
                try:
                    # Read last 50 lines efficiently
                    with open('solver.log', 'r') as f:
                        lines = f.readlines()[-50:]
                        for line in reversed(lines):
                            if "SA epochs explored locally:" in line:
                                m = re.search(r'\[(.*?)s\] SA epochs explored locally: (\d+)', line)
                                if m:
                                    time_el = float(m.group(1))
                                    epochs = int(m.group(2))
                                    break
                except Exception:
                    pass
            
            # Since each epoch checks roughly ~45 depth calculations (pairs of elements)
            # The speed is epochs / time
            speed = (epochs / time_el) if time_el > 0 else 0
            
            data = {
                'status': state,
                'epochs': epochs,
                'time': time_el,
                'speed': round(speed, 2)
            }
            self.wfile.write(json.dumps(data).encode())
        else:
            return super().do_GET()

    def do_POST(self):
        if self.path == '/api/start':
            subprocess.run(['./manage_solver.sh', 'start'])
        elif self.path == '/api/pause':
            subprocess.run(['./manage_solver.sh', 'pause'])
        elif self.path == '/api/resume':
            subprocess.run(['./manage_solver.sh', 'resume'])
        elif self.path == '/api/stop':
            subprocess.run(['./manage_solver.sh', 'stop'])
        else:
            self.send_response(404)
            self.end_headers()
            return
            
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(json.dumps({'status': 'ok'}).encode())

# Do not log every request to terminal to keep it clean
def log_message(self, format, *args):
    pass
CustomHandler.log_message = log_message

if __name__ == "__main__":
    with TCPServer(("", PORT), CustomHandler) as httpd:
        print(f"Solver UI started at http://localhost:{PORT}")
        httpd.serve_forever()
