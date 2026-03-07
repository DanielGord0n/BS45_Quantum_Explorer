import os
import re
import json
import subprocess
import time
from http.server import SimpleHTTPRequestHandler
from socketserver import TCPServer

PORT = 8080
DIRECTORY = "public"

def parse_solver_log(log_file):
    """Parse a solver log file and return epochs, time, and speed."""
    epochs = 0
    time_el = 0.0
    speed = 0.0
    
    if os.path.exists(log_file):
        try:
            with open(log_file, 'r') as f:
                lines = f.readlines()[-50:]
                for line in reversed(lines):
                    if "SA epochs explored locally:" in line:
                        m = re.search(r'\[(.*?)s\] SA epochs explored locally: (\d+) Speed: ([\d\.]+)', line)
                        if m:
                            time_el = float(m.group(1))
                            epochs = int(m.group(2))
                            speed = float(m.group(3))
                            break
        except Exception:
            pass
    
    return epochs, time_el, speed

def get_solver_status(manage_script):
    """Get the status of a solver using its management script."""
    try:
        out = subprocess.run([manage_script, 'status'], capture_output=True, text=True).stdout
        if 'RUNNING' in out:
            return 'RUNNING'
        elif 'PAUSED' in out:
            return 'PAUSED'
        else:
            return 'STOPPED'
    except Exception:
        return 'STOPPED'

class CustomHandler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def do_GET(self):
        if self.path == '/':
            self.path = '/index.html'
            return super().do_GET()
        elif self.path == '/api/status':
            self._send_status('./manage_solver.sh', 'solver.log')
        elif self.path == '/api/bs43/status':
            self._send_status('./manage_bs43.sh', 'bs43_repro.log')
        else:
            return super().do_GET()

    def _send_status(self, manage_script, log_file):
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        
        state = get_solver_status(manage_script)
        epochs, time_el, speed = parse_solver_log(log_file)
        
        data = {
            'status': state,
            'epochs': epochs,
            'time': time_el,
            'speed': round(speed, 2)
        }
        self.wfile.write(json.dumps(data).encode())

    def do_POST(self):
        # BS(45) controls
        if self.path == '/api/start':
            subprocess.run(['./manage_solver.sh', 'start'])
        elif self.path == '/api/pause':
            subprocess.run(['./manage_solver.sh', 'pause'])
        elif self.path == '/api/resume':
            subprocess.run(['./manage_solver.sh', 'resume'])
        elif self.path == '/api/stop':
            subprocess.run(['./manage_solver.sh', 'stop'])
        # BS(43) controls
        elif self.path == '/api/bs43/start':
            subprocess.run(['./manage_bs43.sh', 'start'])
        elif self.path == '/api/bs43/pause':
            subprocess.run(['./manage_bs43.sh', 'pause'])
        elif self.path == '/api/bs43/resume':
            subprocess.run(['./manage_bs43.sh', 'resume'])
        elif self.path == '/api/bs43/stop':
            subprocess.run(['./manage_bs43.sh', 'stop'])
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
        print(f"  BS(45): http://localhost:{PORT}/")
        print(f"  BS(43): http://localhost:{PORT}/bs43.html")
        httpd.serve_forever()
