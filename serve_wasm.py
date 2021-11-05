import BaseHTTPServer, SimpleHTTPServer, sys, webbrowser as b
HOST, PORT = ('localhost', 8888)
class RequestHandler (SimpleHTTPServer.SimpleHTTPRequestHandler):
    def end_headers (self):
        self.send_header('Access-Control-Allow-Origin', '*')
        SimpleHTTPServer.SimpleHTTPRequestHandler.end_headers(self)
RequestHandler.extensions_map['.wasm'] = 'application/wasm'
if len(sys.argv) > 1: b.open_new_tab('http://%s:%d/%s' % (HOST, PORT, sys.argv[1]))
BaseHTTPServer.HTTPServer((HOST, PORT), RequestHandler).serve_forever()
