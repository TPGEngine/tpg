#library(caTools)
#library(zoo)
#options(scipen=-999)

args <- commandArgs(trailingOnly = TRUE);

inFile=args[1]
ylabel=args[2]
winSize=as.numeric(args[3])
outFile=args[4]
maxy=as.numeric(args[5])


no_col <- max(count.fields(inFile))
tbl <- read.table(inFile,fill=TRUE,col.names=1:no_col)
tbl_m = data.matrix(tbl)
medians <- apply(tbl_m, 2, median,na.rm=TRUE)

pdf(paste(outFile,'.pdf',sep=""), width = 6, height = 6)
#png(paste(outFile,'.png',sep=""), width = 600, height = 600)
rangex <- range(0,ncol(tbl_m))
if (maxy > 0){
   rangey <- range(0,maxy)
} else  {
   rangey <- range(min(tbl_m,na.rm=TRUE),max(tbl_m,na.rm=TRUE))
   # rangey <- range(-100,0)
   #rangey <- range(min(medians,na.rm=TRUE),max(tbl_m,na.rm=TRUE))
}
# rangey = range(-1, 0.2)
par(bty="n", cex=1, cex.axis=1, las=1, mar=c(5, 6, 4, 4) + 0.1, lwd=1)

plot(rangex, rangey, type="n", col="black", ann=FALSE, axes=FALSE, ylim=rangey)  

grid(lwd=1, col=1)

rowColours <- rainbow(nrow(tbl_m))
for (x in c(1:nrow(tbl_m))){
  #lines(runmean(tbl_m[x,],1),y=NULL,lty=1,lwd=1,col=rowColours[x])
   lines(tbl_m[x,],y=NULL,lty=1,lwd=1,col=rowColours[x])
}
#lines(runmean(medians,winSize),y=NULL,lty=1,col="black",lwd=2)
#lines(medians,y=NULL,lty=1,col="black",lwd=2)

#abline(v = 2000, col="red", lwd=1, lty=2)
axis(1)
axis(2)
#title(args[1])
title(xlab="Generation")
title(ylab=ylabel,line=5)
invisible(dev.off())
