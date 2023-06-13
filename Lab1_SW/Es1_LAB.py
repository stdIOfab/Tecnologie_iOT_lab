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
                    dataDict["targetValue"] = self.celsiusToKelvin(float(params["value"]))
                if params["originalUnit"].upper() == 'C' and params["targetUnit"].upper() == 'F' :
                    dataDict["targetValue"] = self.celsiusToFahreheit(float(params["value"]))
                if params["originalUnit"].upper() == 'K' and params["targetUnit"].upper() == 'C' :
                    dataDict["targetValue"] = self.kelvinToCelsius(float(params["value"]))
                if params["originalUnit"].upper() == 'K' and params["targetUnit"].upper() == 'F' :
                    dataDict["targetValue"] = self.kelvinToFahrenheit(float(params["value"]))
                if params["originalUnit"].upper() == 'F' and params["targetUnit"].upper() == 'C' :
                    dataDict["targetValue"] = self.fahrenheitToCelsius(float(params["value"]))
                if params["originalUnit"].upper() == 'F' and params["targetUnit"].upper() == 'K' :
                    dataDict["targetValue"] = self.fahrenheitToKelvin(float(params["value"]))
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

    def POST(self,*uri,**params ):

    def celsiusToKelvin(self, temp):
        return temp + 273.15

    def kelvinToCelsius(self, temp):
        return temp - 273.15

    def celsiusToFahreheit(self, temp):
        return round((temp * 9 / 5) + 32, 2)

    def fahrenheitToCelsius(self, temp):
        return round((temp - 32) * 5 / 9, 2)

    def fahrenheitToKelvin(self, temp):
        return round(((temp - 32) * 5 / 9) + 273.15, 2)

    def kelvinToFahrenheit(self, temp):
        return round(((temp - 273.15) * 9 / 5) + 32, 2)

if __name__ == '__main__':
    conf={
        '/':{
                'request.dispatch':cherrypy.dispatch.MethodDispatcher(),
                'tool.session.on':True
            }}
    cherrypy.tree.mount(TempConverter(), '/', conf)
    cherrypy.engine.start()
    cherrypy.engine.block()