#!/usr/bin/env python

from http.server import HTTPServer, SimpleHTTPRequestHandler
from pathlib import Path
import time

host_name = "localhost"
server_port = 8080

Handler = SimpleHTTPRequestHandler


def run():
    server_address = (host_name, server_port)
    httpd = HTTPServer(server_address, Handler)
    print("Serving at ", server_address)
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass

    httpd.server_close()
    print("Server stopped.")


if __name__ == '__main__':
    run()
