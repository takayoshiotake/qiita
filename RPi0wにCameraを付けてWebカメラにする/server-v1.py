#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import http.server
import socketserver
import json

import subprocess
import sys

PORT = 8080


class RequestHandler(http.server.SimpleHTTPRequestHandler):
    protocol_version = 'HTTP/1.1'
    
    def do_HEAD(self):
        self.send_error(405)
        

    def do_POST(self):
        if self.path == '/api' or self.path.startswith('/api/'):
            self.do_api('POST')
            return
        self.send_error(404)


    def do_GET(self):
        if self.path == '/api' or self.path.startswith('/api/'):
            self.do_api('GET')
            return
        super().do_GET()


    def do_api(self, method):
        if method == 'GET' and self.path == '/api/camera.liveview.jpg':
            subargs = 'raspistill -w 640 -h 480 -q 20 -o - -t 1 -n'
            try:
                res = subprocess.run(subargs, shell=True, stdout=subprocess.PIPE).stdout
                self.send_response(200)
                self.send_header('Content-Type', 'image/jpeg')
                self.send_header('Content-Length', len(res))
                self.end_headers()
                self.wfile.write(res)
            except:
                self.send_error(503)
        else:
            self.send_error(404)


    def send_response(self, code, message=None):
        super().send_response_only(code, message)


    def send_error(self, code, message=None, explain=None):
        try:
            shortmsg, _ = self.responses[code]
        except KeyError:
            shortmsg, longmsg = '???', '???'
        if message is None:
            message = shortmsg
        self.log_error('code {0}, message {1}'.format(code, message))
        response = {
            'code': code,
            'message': message
        }
        response_bytes = json.dumps(response).encode('utf-8')
        self.send_response(code, message)
        self.send_header('Content-Type', 'application/json; charset=utf-8')
        self.send_header('Content-Length', len(response_bytes))
        self.end_headers()
        self.wfile.write(response_bytes)


class ThreadingServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    pass


if __name__ == '__main__':
    os.chdir('./web')
    socketserver.TCPServer.allow_reuse_address = True
    server_address = ("", PORT)
    
    httpd = ThreadingServer(server_address, RequestHandler)
    print("serving at port", PORT)
    httpd.serve_forever()
