import pandas 

a = pandas.DataFrame(columns=["dose", "proportion"])

b = pandas.DataFrame({"dose":10, "proportion":20})

a.append(b)

print(a)