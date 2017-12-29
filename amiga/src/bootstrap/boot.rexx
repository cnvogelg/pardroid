/* boot.rexx */
arg outf inf
if inf="" then do
 inf="par:"
 open('0',inf,'W')
 writech('0',"RXBT")
 close('0')
end
if outf="" then outf="ram:s2"
say "s1" inf outf
open('1',inf)
h=readch('1',8)
o="KNOK"
if substr(h,1,4)~=o then do
 say o
 exit 10
end
s=c2d(substr(h,5,2))
z=c2d(substr(h,7,2))
b=1024
l=s
say "size" s
open('2',outf,'W')
do while l>0
 r=l
 if r>b then r=b
 l=l-r
 say l
 d=readch('1',r)
 writech('2',d)
end
close('1'); close('2')
say "check"
open('1',outf)
l=s
y=0
m=65536
do while l>0
 r=l
 if r>b then r=b
 l=l-r
 say l
 d=readch('1',r)
 do i=1 to r by 2
  v=c2d(substr(d,i,2))
  y=y+v
  if y>=m then y=y-m
 end
end
close('1')
if y ~= z then do
 say "CHKSUM!" d2x(y) d2x(z)
 exit 20
end
say "s2"
address command outf
exit
