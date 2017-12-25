/* boot.rexx */
arg inf outf
if inf="" then inf="par:"
if outf="" then outf="ram:stage2"
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
 parse arg v,cs
 hi=c2d(substr(cs,1,2))
 lo=c2d(substr(cs,3,2))
 vhi=c2d(substr(v,1,2))
 vlo=c2d(substr(v,3,2))
 lo=vlo+lo
 hi=vhi+hi
 max=65536
 if lo>=max then do
  hi=hi+1
  lo=lo-max
 end
 if hi>=max then do
  hi=hi-max
 end
 return d2c(hi,2)||d2c(lo,2)
