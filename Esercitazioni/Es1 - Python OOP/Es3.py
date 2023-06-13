from random import shuffle

class Deck :

    def __init__(self):
        self._deck = [ ]
        self._hand = [ ]
        for suit in Card.availableSuits:
            for val in Card.availableValues:
                self._deck.append(Card(suit, val))

    def __str__(self):
        return "Card deck containing {} cards".format(len(self._deck))

    def deal(self, n):
        if len(self._deck) >= n :
            self._hand.clear()
            for i in range(1, n+1, 1):
                self._hand.append(self._deck[len(self._deck) - 1])
                self._deck.remove(self._deck[len(self._deck) - 1])
            return self._hand
        else:
            raise ValueError("Not enough cards in the deck")

    def shuffle(self):
        shuffle(self._deck)

class Card :

    availableSuits = ["Hearts", "Diamonds", "Clubs", "Spades"]
    availableValues = ["A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"]

    def __init__(self, suit, val):
        if suit in Card.availableSuits:
            self._suit = suit
            if val in Card.availableValues:
                self._val = val
            else:
                raise ValueError(f"Invalid value {val}")
        else:
            raise ValueError(f"Invalid suit {suit}")

    def __str__(self):
        return "{} of {}".format(self._val, self._suit)