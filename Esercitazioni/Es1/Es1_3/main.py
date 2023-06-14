from Exercise_3 import Card, Deck

if __name__ == '__main__':
    d = Deck()
    print(d)
    # we did not shuffle, dealt cards will be in order
    hand = d.deal(5)
    for c in hand:
        print(c)
    print(d)
    d.shuffle()
    hand = d.deal(5)
    for c in hand:
        print(c)
    print(d)
    # we don't have 50 cards in the deck
    try:
        hand = d.deal(50)
    except ValueError as err:
        print("Got value error with message:", err)
    # cards constructors should have a minimal error checking
    try:
        c = Card('Sharks', 2)
    except ValueError as err:
        print("Got value error with message:", err)
