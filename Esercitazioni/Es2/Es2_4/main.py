import json
import requests


class Client(object):
    def __init__(self, apiKey):
        self.baseURL = 'http://api.exchangeratesapi.io/v1/'
        self.APIKey = apiKey
        self.baseDict = {'E': 'EUR', 'U': 'USD', 'P': 'GBP'}

    def handler(self, command):
        if command == 'latest':
            self.getLatest()
        if command == 'history':
            self.getHistory()

    def getLatest(self):
        base = self.baseDict[str(input('Which base currency you want:\nE:Euro\nU:USD\nP:GBP\n'))]
        r = requests.get(self.baseURL + 'latest?base={}&access_key={}'.format(base, self.APIKey))
        print(json.dumps(r.json(), indent=4))

    def getHistory(self):
        x = input('Type D for a day and I for and interval\n')
        if x == 'D':
            year = input('Write the year\n')
            month = input('Write the month\n')
            day = input('Write the day\n')
            r = requests.get(self.baseURL + '{}-{}-{}?&access_key={}'.format(year, month, day, self.APIKey))
            print(json.dumps(r.json(), indent=4))
        if x == 'I':
            s_year = input('Write the starting year\n')
            s_month = input('Write the starting month\n')
            s_day = input('Write the starting day\n')
            e_year = input('Write the ending year\n')
            e_month = input('Write the ending month\n')
            e_day = input('Write the ending day\n')
            self.baseURL + 'history?start_at={}-{}-{}&end_at={}-{}-{}&access_key={}'.format(s_year, s_month, s_day,
                                                                                            e_year, e_month, e_day,
                                                                                            self.APIKey)
            r = requests.get(
                self.baseURL + 'history?start_at={}-{}-{}&end_at={}-{}-{}&access_key={}'.format(s_year, s_month, s_day,
                                                                                                e_year, e_month, e_day,
                                                                                                self.APIKey))
            print(json.dumps(r.json(), indent=4))


def main():
    apiKey = '99da1e51e118a237b062b63e65e39bdf'
    c = Client(apiKey)
    while True:
        command = input(
            'Available command:\nlatest:latest change rate\nhistory: historic exchange rates\nquit:exit\n')
        if command == 'quit':
            break
        elif command == 'latest' or command == 'history':
            c.handler(command)
        else:
            print('Wrong command')


if __name__ == '__main__':
    main()
