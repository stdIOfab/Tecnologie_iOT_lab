import time

import cherrypy, json



class TempConverter(object) :
    exposed = True

    def POST(self, *uri):
        output = dict()
        body = cherrypy.request.body.read()
        json_object = json.loads(body)
        output["values"] = json_object["values"]
        output["targetValues"] = [ ]
        if json_object["originalUnit"] == 'C':
            if json_object["targetUnit"] == 'K':
                for el in json_object["values"]:
                    output["targetValues"].append(self.celsiusToKelvin(el))
            if json_object["targetUnit"] == 'F':
                for el in json_object["values"]:
                    output["targetValues"].append(self.celsiusToFahreheit(el))
        if json_object["originalUnit"] == 'K':
            if json_object["targetUnit"] == 'C':
                for el in json_object["values"]:
                    output["targetValues"].append(self.kelvinToCelsius(el))
            if json_object["targetUnit"] == 'F':
                for el in json_object["values"]:
                    output["targetValues"].append(self.kelvinToFahrenheit(el))
        if json_object["originalUnit"] == 'F':
            if json_object["targetUnit"] == 'K':
                for el in json_object["values"]:
                    output["targetValues"].append(self.fahrenheitToKelvin(el))
            if json_object["targetUnit"] == 'C':
                for el in json_object["values"]:
                    output["targetValues"].append(self.fahrenheitToCelsius(el))
        output["originalUnit"] = json_object["originalUnit"]
        output["targetUnit"] = json_object["targetUnit"]
        output["timestamp"] = round(time.time())
        with open('data.json', 'w+') as json_file:
            json.dump(output, json_file)
        with open('data.json', 'r') as f:
            json_data = json.load(f)
            return json.dumps(json_data, indent=2)

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