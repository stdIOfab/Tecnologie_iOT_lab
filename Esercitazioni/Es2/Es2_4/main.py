import json
import requests
import datetime


def isValidDateTillNow(year, month, day):
    #try:
    day_count_for_month = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    if int(year) % 4 == 0 and (int(year) % 100 != 0 or int(year) % 400 == 0):
        day_count_for_month[2] = 29
    #except Exception:
        #return False
    return 1 <= int(month) <= 12 and 1 <= int(day) <= day_count_for_month[int(month)] and datetime.datetime.now() > datetime.datetime(int(year), int(month), int(day))


class Client(object):
    def __init__(self, apiKey):
        self.requestURL = 'http://api.exchangeratesapi.io/v1/'
        self.apiKey = apiKey
        self.baseDict = {'E': 'EUR', 'U': 'USD', 'P': 'GBP'}

    def handler(self, command):
        if command == 'latest':
            self.getLatest()
        if command == 'history':
            self.getHistory()

    def getLatest(self):
        try:
            base = self.baseDict[str(input('Which base currency you want:\nE:Euro\nU:USD\nP:GBP\n'))]
        except KeyError:
            print('Wrong input')
            return
        r = requests.get(self.requestURL + 'latest?base={}&access_key={}'.format(base, self.apiKey))
        print(json.dumps(r.json(), indent=4))

    def getHistory(self):
        x = input('Type D for a day and I for and interval\n')
        if x == 'D':
            year = input('Write the year\n')
            month = input('Write the month\n')
            day = input('Write the day\n')
            if not isValidDateTillNow(year, month, day):
                print('Wrong date')
                return
            r = requests.get(self.requestURL + '{}-{}-{}?&access_key={}'.format("%.4d"%int(year), "%.2d"%int(month), "%.2d"%int(day), self.apiKey))
            print(json.dumps(r.json(), indent=4))
        elif x == 'I':
            s_year = input('Write the starting year\n')
            s_month = input('Write the starting month\n')
            s_day = input('Write the starting day\n')
            e_year = input('Write the ending year\n')
            e_month = input('Write the ending month\n')
            e_day = input('Write the ending day\n')
            self.requestURL + 'history?start_at={}-{}-{}&end_at={}-{}-{}&access_key={}'.format(s_year, s_month, s_day,
                                                                                               e_year, e_month, e_day,
                                                                                               self.apiKey)
            r = requests.get(
                self.requestURL + 'history?start_at={}-{}-{}&end_at={}-{}-{}&access_key={}'.format(s_year, s_month, s_day,
                                                                                                   e_year, e_month, e_day,
                                                                                                   self.apiKey))
            print(json.dumps(r.json(), indent=4))
        else:
            print('Wrong input')


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
            print('Wrong command, try again.')


if __name__ == '__main__':
    main()
