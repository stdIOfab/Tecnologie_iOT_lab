import cherrypy, json

class TempConverter(object) :
    exposed = True
    def GET(self,*uri):
        dataDict = dict()
        if len(uri) == 4 and uri[0] == 'converter' :
            try:
                dataDict["originalValue"] = uri[1]
                dataDict["originalUnit"] = uri[2].upper()
                if uri[2].upper() == 'C' and uri[3].upper() == 'K' :
                    dataDict["targetValue"] = self.celsiusToKelvin(float(uri[1]))
                if uri[2].upper() == 'C' and uri[3].upper() == 'F' :
                    dataDict["targetValue"] = self.celsiusToFahreheit(float(uri[1]))
                if uri[2].upper() == 'K' and uri[3].upper() == 'C' :
                    dataDict["targetValue"] = self.kelvinToCelsius(float(uri[1]))
                if uri[2].upper() == 'K' and uri[3].upper() == 'F' :
                    dataDict["targetValue"] = self.kelvinToFahrenheit(float(uri[1]))
                if uri[2].upper() == 'F' and uri[3].upper() == 'C' :
                    dataDict["targetValue"] = self.fahrenheitToCelsius(float(uri[1]))
                if uri[2].upper() == 'F' and uri[3].upper() == 'K' :
                    dataDict["targetValue"] = self.fahrenheitToKelvin(float(uri[1]))
                dataDict["targetUnit"] = uri[3].upper()
                with open('data.json', 'w+') as json_file :
                    json.dump(dataDict, json_file)
                with open('data.json', 'r') as f:
                    json_data = json.load(f)
                    return json.dumps(json_data, indent=2)
            except ValueError:
                raise cherrypy.HTTPError(400, "Bad Request, wrong type of arguments :(")
        else :
            raise cherrypy.HTTPError(400, "Bad Request, wrong number of arguments :(")

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
                'request.show_tracebacks':False,
                'request.dispatch':cherrypy.dispatch.MethodDispatcher(),
                'tool.session.on':True
            }}
    cherrypy.tree.mount(TempConverter(), '/', conf)
    cherrypy.engine.start()
    cherrypy.engine.block()