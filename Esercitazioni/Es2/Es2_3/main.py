import cherrypy, json


class MyWebService(object):
    exposed = True

    def GET(self, *uri, **params):
        reverseParam = ""
        if params != {}:
            for key in params:
                valueList = []
                for i in range(len(params[key]) - 1, -1, -1):
                    valueList.append(params[key][i])
                params[key] = reverseParam.join(valueList)
        return str(params)

    def PUT(self, *uri, **params):
        printKeys = 'The keys are '
        printValues = ' and the values are '
        convertedDict = json.loads(cherrypy.request.body.read())
        return printKeys + str(list(convertedDict.keys())) + printValues + str(list(convertedDict.values()))


if __name__ == '__main__':
    conf = {
        '/': {
            'request.dispatch': cherrypy.dispatch.MethodDispatcher(),
            'tool.session.on': True
        }}
    cherrypy.tree.mount(MyWebService(), '/', conf)
    cherrypy.engine.start()
    cherrypy.engine.block()
