availableRank = ['A', 2, 3, 4, 5, 6, 7, 8, 9, 10, 'J', 'Q', 'K']


class Card:
    def __init__(self, suit, rank):
        if suit not in ['Hearts', 'Diamonds', 'Clubs', 'Spades']:
            raise ValueError(f"Invalid suit {suit}")
        if rank not in availableRank:
            raise ValueError(f"Invalid rank {rank}")
        self.suit = suit
        self.rank = rank

    def __str__(self):
        return f"{self.rank} of {self.suit}"


class Deck:
    def __init__(self):
        self.cards = []
        for suit in ['Clubs', 'Diamonds', 'Hearts', 'Spades']:
            for rank in availableRank:
                self.cards.append(Card(suit, rank))

    def __str__(self):
        return f"Card Deck containing {len(self.cards)} cards"

    def deal(self, n):
        if n > len(self.cards):
            raise ValueError(f"Not enough cards in the deck, only {len(self.cards)} cards left")
        return [self.cards.pop() for _ in range(n)]

    def shuffle(self):
        import random
        random.shuffle(self.cards)
