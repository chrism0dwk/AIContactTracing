# ./sellke.R
#
# Copyright 2012 Chris Jewell <chrism0dwk@gmail.com>
#
# This file is part of InFER.
#
# InFER is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# InFER is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with InFER.  If not, see <http://www.gnu.org/licenses/>. 

createSellke <- function(infecs,pressures) {

  infecPress = pressures[infecs]
  nonInfecPress = pressures[-infecs]

  uniquePress = unique(pressures)
  uniquePress = sort(uniquePress) # Reorder
  
  heights = rep(0,length(uniquePress))

  myProd = 1
  varSum = 0

  for(i in 1:length(uniquePress)) {

    isInfecEvent = length(infecPress[infecPress==uniquePress[i]])

    if(isInfecEvent == 0) {
      heights[i] = myProd
    }
    else {
      # Calculate densities for infecteds
      numAtRisk = length(infecPress[infecPress>=uniquePress[i]])

      # Calculate densities for non-infecteds
      numAtRisk = numAtRisk + length(nonInfecPress[nonInfecPress>=uniquePress[i]])
    
      myProd = myProd * (numAtRisk - isInfecEvent) / numAtRisk

      heights[i] = myProd
    }
    

    
  }

  return(list(pressure=uniquePress,density=heights))
}





createSellke2 <- function(sellkeSet) {

  require(survival);
  
  timePeriodToks <- unlist(strsplit(sellkeSet,";",fixed=TRUE))
  myTuples <- unlist(strsplit(timePeriodToks,",",fixed=TRUE))
      
  pressures <- numeric(length(myTuples))
  censored <- numeric(length(myTuples))
  for(i in 1:length(myTuples)) {
      pair <- unlist(strsplit(myTuples[i],":",fixed=TRUE))
      pressures[i] <- as.double(pair[1])
      censored[i] <- as.integer(pair[2])
    }

  return(Surv(pressures,censored))
}



ssSellkeNew <- function(sellkeDir,outputFilename,sellkeRep)
{
  myFiles <- dir(path=sellkeDir,pattern=".sellke$")
      
  pdf(file=outputFilename)
  
  for(sellkeFileName in myFiles) {
    cat("Sellke file:", sellkeFileName,"\n")
	
    fc <- file(sellkeFileName,"r")
#for(k in 1:sellkeRep) {
      myLine <- readLines(fc,n=1)
      	
      mySurv <- createSellke2(myLine)
      myFit <- survfit(mySurv~1)
      x <- seq(0,max(myFit$time),len=1000)
      y <- dexp(x)
      plot(myFit,main=sellkeFileName,mark.time=F)
      lines(x,y,col="red")
#    }
    close(fc)
	
  }
  
}


ssSellke <- function(confPath,sellkePrefix,outputFilename,sellkeRep)
{
  require(survival)
      
  myFiles <- dir(path=confPath,pattern=".conf$")

      x <- seq(0,5,len=1000)
      y <- dexp(x)
      
      pdf(file=outputFilename)
  
      for(confFileName in myFiles) {
      cat("Conf file:",confFileName,"\n")
    
	epiFile <- scan(paste(confPath,"/",confFileName,sep="",what=character(0)),sep="\n",what=character(0))
	epiLine <- grep("^EpiData",epiFile)
	if(length(epiLine) != 1) {
      cat("Problem reading config file ",confFile,". Please check")
	  return()
	}
	epiFileName <- unlist(strsplit(epiFile[epiLine],"="))[2]
	    epiFileName <- paste(epiFileName,".ipt",sep="")
	    cat("Epidemic file",epiFileName,"\n")
	    epiFile <- read.table(epiFileName)
    
	    confNumber <- unlist(strsplit(confFileName,"\\."))[2]
    
	    sellkeFileName <- paste(sellkePrefix,".",confNumber,".sellke",sep="")
	    cat("Sellke file:",sellkeFileName,"\n")
	    sellkeFile <- scan(sellkeFileName,sep=",")

            censored <- rep(0,length(sellkeFile)/sellkeRep)
	    for(i in 1:length(epiFile[,1])) {
	      censored[epiFile[i,1]]=1
	    }
	    
	    plot(0,xlim=c(0,max(sellkeFile)),ylim=c(0,1),type="n",main=paste(confFileName,": ",length(epiFile[,1])," infected",sep=""))
	    
	   for(i in 1:sellkeRep) {
	     mySurv <- Surv(sellkeFile[((i-1)*8636+1):(i*8636)],censored)
	     mySurvfit <- survfit(mySurv ~ 1)
	     lines(mySurvfit) # plot K-M estimate
	     lines(mySurvfit$time,mySurvfit$lower,type="s",col="grey")
	     lines(mySurvfit$time,mySurvfit$upper,type="s",col="grey")
	    }
	    lines(x,y,col="red")
    
      }
      dev.off()
}



plotSellkeResid <- function(confPath,sellkePrefix,outputFilename,sellkeRep)
{
  myFiles <- dir(path=confPath,pattern=".conf$")

      x <- seq(0,5,len=1000)
      y <- dexp(x)
      
      pdf(file=outputFilename)
  
      for(confFileName in myFiles) {
    cat("Conf file:",confFileName,"\n")
    
	epiFile <- scan(paste(confPath,"/",confFileName,sep="",what=character(0)),sep="\n",what=character(0))
	epiLine <- grep("^EpiData",epiFile)
	if(length(epiLine) != 1) {
      cat("Problem reading config file ",confFile,". Please check")
	  return()
	}
	epiFileName <- unlist(strsplit(epiFile[epiLine],"="))[2]
	    epiFileName <- paste(epiFileName,".ipt",sep="")
	    cat("Epidemic file",epiFileName,"\n")
	    epiFile <- read.table(epiFileName)
    
	    confNumber <- unlist(strsplit(confFileName,"\\."))[2]
    
	    sellkeFileName <- paste(sellkePrefix,".",confNumber,".sellke",sep="")
	    cat("Sellke file:",sellkeFileName,"\n")
	    sellkeFile <- scan(sellkeFileName,sep=",")

	    for(i in 1:sellkeRep) {
	       mySellke <- createSellke(epiFile[,1],sellkeFile[((i-1)*8636+1):(i*8636)])
	       normResid <- (mySellke$density - dexp(mySellke$pressure))/mySellke$pressure
	       normResid <- normResid[-1] # Get rid of the -Inf at the beginning
	       residDens <- density(normResid)
	       xmin=min(residDens$x); xmax=max(residDens$x); ymin=min(residDens$y); ymax=max(residDens$y)
	       x <- seq(xmin,xmax,len=1000)
	       y <- dnorm(x)
	       ymax = max(ymax,max(y))
	       plot(x,y,xlim=c(xmin,xmax),ylim=c(ymin,ymax),col="red",type="l",main=paste(confFileName,": ",length(epiFile[,1])," infected",sep=""))
	       lines(density(normResid),col="black")
	    }
    
      }
      dev.off()
}
