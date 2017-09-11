

## Trida reprezentujici bod na krivce
class EC_Point:

    x = None
    y = None

    def __init__(self, x, y):
        self.x = x
        self.y = y

##------------------------------------------------------------------------------
""" Funkce provede vypocet podle Male Fermatovy vety (p je prvocislo) """
def invertModulo(number, p):

    result = pow(number, p-2, p)
    return result

##------------------------------------------------------------------------------
""" Funkce provede vypocet modula v dane zbytkove tride p """
def modulo(number, p):

    result = number + p if number < 0 else number % p
    return result

##------------------------------------------------------------------------------
""" Funkce vypocte inverzni souradnici pro dany bod ve zbytkove tride p """
def inversCoordinate(number, p):

    result = modulo(-number, p) if number > 0 else modulo(number, p)
    return result

##------------------------------------------------------------------------------
""" Funkce provede nasobeni bodu na EC (Elliptic Curve) skalarem (2P)  """
def pointDoubling(P, R, a, p):

    citatel = modulo(3*(P.x*P.x) + a, p)
    jmenovatel = invertModulo(modulo(2*P.y, p), p)

    lambd = (citatel*jmenovatel) % p

    Rx = modulo((lambd*lambd - 2*P.x), p) % p
    Ry = modulo(lambd*modulo(P.x - Rx, p) - P.y, p) % p

    R.x = Rx
    R.y = Ry

##------------------------------------------------------------------------------
""" Funkce provede scitani dvou bodu na EC (Elliptic Curve) """
def pointAddition(P, Q, R, a, p):

    # Bod Q lezi v nekonecnu
    if Q.x == float("inf") and Q.y == float("inf"):
        R.x = P.x
        R.y = P.y

    # Bod P lezi v nekonecnu
    elif P.x == float("inf") and P.y == float("inf"):
        R.x = Q.x
        R.y = Q.y

    # Bod Q je inverznim bodem k P
    elif P.x == Q.x and Q.y == inversCoordinate(P.y, p):
        R.x = float("inf")
        R.y = float("inf")

    # Bod Q a P jsou totozne
    elif P.x == Q.x and P.y == Q.y:
        pointDoubling(P, R, a, p)

    # Bod Q a P jsou rozdilne
    else:
        citatel = modulo(Q.y - P.y, p)
        jmenovatel = invertModulo(modulo(Q.x - P.x, p), p)

        lambd = (citatel*jmenovatel) % p

        Rx = modulo((lambd*lambd - P.x - Q.x), p) % p
        Ry = modulo(lambd*modulo(P.x - Rx, p) - P.y, p) % p

        R.x = Rx
        R.y = Ry

##------------------------------------------------------------------------------
##------------------------------------------------------------------------------
##------------------------------------------------------------------------------
if __name__ == '__main__':

    Fp = int("0xffffffff00000001000000000000000000000000ffffffffffffffffffffffff", 16)
    a = int("-0x3", 16)
    b = int("0x5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b", 16)
    xG = int("0x6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296", 16)
    yG = int("0x4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5", 16)
    xPublicKey = int("0x52910a011565810be90d03a299cb55851bab33236b7459b21db82b9f5c1874fe", 16)
    yPublicKey = int("0xe3d03339f660528d511c2b1865bcdfd105490ffc4c597233dd2b2504ca42a562", 16)

    # Hodnota b neni potreba, jelikoz EC je grupa, jsou jeji operace uzavrene
    # Je-li pocatecni bod G zadan vzdy korektne, neni duvod kontrolovat, zda lezi na krivce

    privateKey = 0;

    pointP = EC_Point(xG, yG)
    pointQ = EC_Point(float("inf"), float("inf"))

    # Vypocet privatniho klice Q = dG (Q = G + G + G + ... + Gn)
    while pointQ.x != xPublicKey and pointQ.y != yPublicKey:
        pointAddition(pointP, pointQ, pointQ, a , Fp)
        privateKey += 1

    print privateKey
