import tornado.httpserver
import tornado.ioloop
import tornado.web
import pprint
import Image
from tesseract import image_to_string
import StringIO

class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.write('<html><body>Send us a file!<br/><form enctype="multipart/form-data" action="/" method="post">'
                   '<input type="file" name="the_file">'
                   '<input type="submit" value="Submit">'
                   '</form></body></html>')

    def post(self):
        self.set_header("Content-Type", "text/plain")
        self.write("You sent a file with name " + self.request.files.items()[0][1][0]['filename'] )
	# make a "memory file" using StringIO, open with PIL and send to tesseract for OCR 
	self.write(image_to_string(Image.open(StringIO.StringIO(self.request.files.items()[0][1][0]['body']))))

application = tornado.web.Application([
    (r"/", MainHandler),
])

if __name__ == "__main__":
    http_server = tornado.httpserver.HTTPServer(application)
    http_server.listen(8888)
    tornado.ioloop.IOLoop.instance().start()

