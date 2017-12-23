/* boot.rexx */
open('1','par:','W')
writech('1','RXBT')
close('1')
file = 'ram:boot'
open('1','par:')
open('2',file,'W')
blk = 1024
data = readch('1', blk)
writech('2', data)
close('1'); close('2')
exit
