from tumqit import syrec

if __name__ == "__main__":
    dummy = syrec.Dummy()
    dummy.setVal(42)
    print(dummy.getVal())
    dummy = syrec.Dummy(42)
