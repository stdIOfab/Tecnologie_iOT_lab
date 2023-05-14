import cherrypy, json

class TempConverter(object) :
    exposed = True
    def GET(self,*uri,**params):
        dataDict = dict()
        if len(uri) != 0 and uri[0] == 'converter' :
            if params != {} and len(params) == 3 :
                dataDict["originalValue"] = params["value"]
                dataDict["originalUnit"] = params["originalUnit"].upper()
                if params["originalUnit"].upper() == 'C' and params["targetUnit"].upper() == 'K' :
                    dataDict["targetValue"] = float(params["value"]) + 273.15
                if params["originalUnit"].upper() == 'C' and params["targetUnit"].upper() == 'F' :
                    dataDict["targetValue"] = round((float(params["value"]) * 9 / 5) + 32, 2)
                if params["originalUnit"].upper() == 'K' and params["targetUnit"].upper() == 'C' :
                    dataDict["targetValue"] = float(params["value"]) - 273.15
                if params["originalUnit"].upper() == 'K' and params["targetUnit"].upper() == 'F' :
                    dataDict["targetValue"] = round(((float(params["value"]) - 273.15) * 9 / 5) + 32, 2)
                if params["originalUnit"].upper() == 'F' and params["targetUnit"].upper() == 'C' :
                    dataDict["targetValue"] = round((float(params["value"]) - 32) * 5 / 9, 2)
                if params["originalUnit"].upper() == 'F' and params["targetUnit"].upper() == 'K' :
                    dataDict["targetValue"] = round(((float(params["value"]) - 32) * 5 / 9) + 273.15, 2)
                dataDict["targetUnit"] = params["targetUnit"].upper()
                with open('data.json', 'w+') as json_file :
                    json.dump(dataDict, json_file)
                with open('data.json', 'r') as f:
                    json_data = json.load(f)
                    return json.dumps(json_data, indent=2)
            else :
                raise cherrypy.HTTPError(400, "Bad Request :(")
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