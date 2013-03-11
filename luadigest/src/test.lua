-- test md5 library

function report(w,s,F)
 print(w,s.."  "..F)
 assert(s==KNOWN)
end

function test(D,known)
 if D==nil then return end
 KNOWN=known
 print""
 print(D.version)
 print""

 assert(io.input(F))
 report("all",D.digest(io.read"*a"),F)

 assert(io.input(F))
 d=D.new()
 while true do
  local c=io.read(1)
  if c==nil then break end
  d:update(c)
 end
 report("loop",d:digest(),F)
 report("again",d:digest(),F)

 assert(io.input(F))
 d:reset()
 while true do
  local c=io.read(math.random(1,16))
  if c==nil then break end
  d:update(c)
 end
 report("reset",d:digest(),F)

 report("known",KNOWN,F)

 local A="hello"
 local B="world"
 local C=A..B
 local a=D.digest(C)
 local b=d:reset():update(C):digest()
 assert(a==b)
 local b=d:reset():update(A,B):digest()
 assert(a==b)
end

F="README"
test(md2,'4cea4d55c69fc5b1c8a2db32a9491114')
test(md4,'2447a1c4ee46a1561742e7fd96dc6a9e')
test(md5,'876e33dfc2a7cd1e46e534d0c00fd4f1')
test(sha1,'e189103170f22bbb25fc6251ef3724c98f2be033')
test(sha224,'37c8c186ed4d7a86249b593deb1741b36c7d6c32b7bd71bf9e67f3f5')
test(sha256,'05ac5c8d98b2a089d87fc911848ba3767d1190dad091dff89010794808ed754a')
test(sha384,'285e6a2ea0ac429146a363e06cc21237579a84aa2529e92605393d03e0fe22d7bbddeecc74e8a3e3e343b11a40a81ab8')
test(sha512,'72adc6409162f81b8ceca616bd5bf931a740c2416ba23a38385f7052e7061bdea66c90fa78847f4df37287e4d072a585d9dfa91428c7f112507fadfee9582300')
test(ripemd160,'f24a89b2be6872480ade3c3b7cd25b6e0fb8212f')
test(mdc2,'94f5b21d365c093dcafa4bfaa9a91b37')

-- eof
