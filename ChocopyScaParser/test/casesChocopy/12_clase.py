class animal(object):
    makes_noise: bool = False

    def make_noise(self: "animal") -> object:
        if self.makes_noise:
            print(self.sound())

    def sound(self: "animal") -> str:
        return "???"


class cow(animal):
    def __init__(self: "cow"):
        self.makes_noise = True
