import cherrypy, json

class TempConverter(object) :
    exposed = True
    def GET(self,*uri,**params):
        dataDict = dict()
        if len(uri) == 4 and uri[0] == 'converter' :
            dataDict["originalValue"] = uri[1]
            dataDict["originalUnit"] = uri[2].upper()
            if uri[2].upper() == 'C' and uri[3].upper() == 'K' :
                dataDict["targetValue"] = float(uri[1]) + 273.15
            if uri[2].upper() == 'C' and uri[3].upper() == 'F' :
                dataDict["targetValue"] = round((float(uri[1]) * 9 / 5) + 32, 2)
            if uri[2].upper() == 'K' and uri[3].upper() == 'C' :
                dataDict["targetValue"] = float(uri[1]) - 273.15
            if uri[2].upper() == 'K' and uri[3].upper() == 'F' :
                dataDict["targetValue"] = round(((float(uri[1]) - 273.15) * 9 / 5) + 32, 2)
            if uri[2].upper() == 'F' and uri[3].upper() == 'C' :
                dataDict["targetValue"] = round((float(uri[1]) - 32) * 5 / 9, 2)
            if uri[2].upper() == 'F' and uri[3].upper() == 'K' :
                dataDict["targetValue"] = round(((float(uri[1]) - 32) * 5 / 9) + 273.15, 2)
            dataDict["targetUnit"] = uri[3].upper()
            with open('data.json', 'w+') as json_file :
                json.dump(dataDict, json_file)
            with open('data.json', 'r') as f:
                json_data = json.load(f)
                return json.dumps(json_data, indent=2)
        else :
            raise cherrypy.HTTPError(404, "Not Found :(")

if __name__ == '__main__':
    conf={
        '/':{
                'request.dispatch':cherrypy.dispatch.MethodDispatcher(),
                'tool.session.on':True
            }}
    cherrypy.tree.mount(TempConverter(), '/', conf)
    cherrypy.engine.start()
    cherrypy.engine.block()