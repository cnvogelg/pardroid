/* boot.rexx */
arg inf outf
if inf="" then inf="par:"
if outf="" then outf="ram:s2"
say "s1" inf outf
open('1',inf)
h=readch('1',16)
o="KNOK"
if substr(h,1,4)~=o then do
 say o
 exit 10
end
s=c2d(substr(h,5,4))
z=substr(h,9,4)
b=1024
l=s
y=d2c(0,4)
open('2',outf,'W')
do while l>0
 r=l
 if r>b then r=b
 l=l-r
 say l
 d=readch('1',r)
 writech('2',d)
 do i=1 to r by 4
  y=f(substr(d,i,4),y)
 end
end
close('1'); close('2')
if y ~= z then do
 say "CHKSUM!"
 exit 20
end
say "s2"
address command outf
exit
f: procedure
 parse arg va,cs
 m=65536
 r=0
 do i=3 to 1 by -2
  v=c2d(substr(va,i,2))
  c=c2d(substr(cs,i,2))+v+r
  if c>=m then do
    r=1
    c=c-m
  end
  e.i=d2c(c,2)
 end
 return e.1||e.3
