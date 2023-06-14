import cherrypy


class MyWebService(object):
    exposed = True

    def __init__(self):
        pass

    def GET(self, *uri, **params):
        reverseParam = ""
        if params != {}:
            for key in params:
                valueList = []
                for i in range(len(params[key]) - 1, -1, -1):
                    valueList.append(params[key][i])
                params[key] = reverseParam.join(valueList)
        return str(params)


if __name__ == '__main__':
    conf = {
        '/': {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
            'tool.session.on': True
        }}
    cherrypy.tree.mount(MyWebService(), '/', conf)
    cherrypy.engine.start()
    cherrypy.engine.block()
