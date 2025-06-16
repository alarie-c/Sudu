# Sudu Language Implemention

Sudu closely translates to "speed" in Mandarin Chinese.
To be implemented as a bytecode interpreter on a virtual machine...
Statically and explicitly typed, with some flexibility.

```
global NAME: str = "World"

function greeting(name: str) -> str
    return "Hello, ${name}"
end

function main()
    print(greeting(NAME))
end
```