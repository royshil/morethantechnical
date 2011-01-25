import tornado.httpserver
import tornado.ioloop
import tornado.web
import pprint
import Image
from tesseract import image_to_string
import StringIO
import os.path
import uuid

class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.write('<html><body>Send us a file!<br/><form enctype="multipart/form-data" action="/" method="post">'
                   '<input type="file" name="the_file">'
                   '<input type="submit" value="Submit">'
                   '</form></body></html>')

    def post(self):
        self.set_header("Content-Type", "text/html")
	self.write("<html><body>")
        self.write("You sent a file with name " + self.request.files.items()[0][1][0]['filename'] +"<br/>" )

	tempname = str(uuid.uuid4()) + ".jpg"
	myimg = Image.open(StringIO.StringIO(self.request.files.items()[0][1][0]['body']))
	myfilename = os.path.join(os.path.dirname(__file__),"static",tempname);
	myimg.save(myfilename)

	self.write("<img src=\"static/" + tempname + "\" /><br/>") 
	self.write(image_to_string(myimg))
	self.write("</body></html>")

# TODO: probably add another handler for NLP issues

settings = {
    "static_path": os.path.join(os.path.dirname(__file__), "static"),
}

application = tornado.web.Application([
    (r"/", MainHandler),
], **settings)

if __name__ == "__main__":
    http_server = tornado.httpserver.HTTPServer(application)
    http_server.listen(8888)
    tornado.ioloop.IOLoop.instance().start()

