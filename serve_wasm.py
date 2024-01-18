#!/usr/bin/env python3

import http.server
import socketserver
import sys
import webbrowser as b

HOST, PORT = ('localhost', 8888)

class RequestHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        super().end_headers()

RequestHandler.extensions_map['.wasm'] = 'application/wasm'

if len(sys.argv) > 1:
    b.open_new_tab('http://%s:%d/%s' % (HOST, PORT, sys.argv[1]))

print('Listening on: http://%s:%d/' % (HOST, PORT))

httpd = socketserver.TCPServer((HOST, PORT), RequestHandler)
httpd.serve_forever()
