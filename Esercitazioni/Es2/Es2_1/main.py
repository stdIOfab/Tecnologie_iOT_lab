import cherrypy


class MyWebService(object):
    exposed = True

    def __init__(self):
        pass

    def GET(self, *uri):
        reverse = ""
        letterList = []
        if len(uri) != 0:
            for i in range(len(uri[0]) - 1, -1, -1):
                letterList.append(uri[0][i])
            reverse = reverse.join(letterList)
            print(uri)
        return reverse


if __name__ == '__main__':
    conf = {
        '/': {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
            'tool.session.on': True
        }}
    cherrypy.tree.mount(MyWebService(), '/', conf)
    cherrypy.engine.start()
    cherrypy.engine.block()
