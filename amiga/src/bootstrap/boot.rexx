/* boot.rexx */
arg inf outf
if inf="" then inf="par:"
if outf="" then outf="ram:s2"
say "stage1" inf outf
/* hdr */
open('1',inf)
hdr=readch('1',16)
knok=substr(hdr,1,4)
if knok ~= "KNOK" then do
 say "no KNOK!"
 exit 10
end
size=c2d(substr(hdr,5,4))
cs=substr(hdr,9,4)
/* data */
blk=1024
left=size
ms=d2c(0,4)
open('2',outf,'W')
do while left>0
 rem=left
 if rem>blk then rem=blk
 left=left-rem
 data=readch('1',rem)
 writech('2',data)
 do i=1 to rem by 4
  ms=f(substr(data,i,4),ms)
 end
end
close('1'); close('2')
if ms ~= cs then do
 say "CHECK SUM ERROR!"
 exit 10
end
say "stage2"
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
