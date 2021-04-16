require("usdt")
provider = usdt.provider("lua", "lmod")

iprobe = provider:probe("foo", "iprobe",
                        "int", "int", "int", "int",
                        "int", "int", "int", "int",
                        "int", "int")

cprobe = provider:probe("foo", "cprobe",
                        "char *", "char *", "char *", "char *",
                        "char *", "char *", "char *", "char *",
                        "char *", "char *")

provider:enable()

if (iprobe:enabled()) then
  iprobe:fire(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)
end
if (cprobe:enabled()) then
  cprobe:fire("a", "b", "c", "d", "e", "f", "g", "h", "i", "j")
end

provider:disable()
