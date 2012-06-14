# ./scratch.r
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

trace.plot <- function(output,density=F,thin=1,lambda=NULL, nu=NULL) {

  mains <- c(expression(beta_0),expression(beta_1),expression(beta_2),expression(beta_3),expression(beta_4),expression(beta_5),expression(beta_6),expression(beta_7),expression(beta_8),expression(beta_9),expression(beta_10),expression(beta_11),expression(beta_12),expression(beta_13),expression(beta_14),expression(beta_15),"Mean I","Occults","Likelihood")
  
  if(density==T) {
    par(mfrow=c(4,5))
    
    if(is.null(lambda) || is.null(nu)) {
      for(i in 1:19) {
        plot(density(output[seq(1,length(output[,1]),by=thin),i]),main=mains[i])
      }
    }
    
    else {
      for(i in 1:16) { # We plot priors on the posteriors
        cat("Density ",i,"\n")
        myDens = density(output[seq(1,length(output[,i]),by=thin),i])
        xMin = min(myDens$x)
        xMax = max(myDens$x)
        cat("Calculating prior...\n")
        x <- seq(xMin,xMax,length=1000)
        prior <- dgamma(x,lambda[i])/nu[i]

        yMin = 0
        yMax = max(myDens$y,prior)

	if(max(prior) < max(myDens$y)) {
	  scaleFactor = max(myDens$y) / max(prior)
	  prior = prior*scaleFactor
        }

        cat("Plotting...\n")
        plot(myDens,main=mains[i],xlim=c(xMin,xMax),ylim=c(yMin,yMax))
        lines(x,prior,col="red")
        cat(i," done\n")
      }
      cat("Parameters done\n")
      for(i in 17:19) {
        plot(density(output[seq(1,length(output[,i]),by=thin),i]),main=mains[i])
      }
    }
  }


      
  else {
    #X11()
    par(mfrow=c(4,5))
    for(i in 1:19) {
      plot(output[seq(1,length(output[,1]),by=thin),i],type="l",main=mains[i])
    }
  }
}











ftrace.plot <- function(output,file,density=F,thin=1) {
  pdf(file)
  if(density==T) {
    par(mfrow=c(4,2))
    for(i in 1:8) {
      plot(density(output[seq(1,length(output[,1]),by=thin),i]))
    }
  }
  else {
    #X11()
    par(mfrow=c(4,2))
    for(i in 1:8) {
      plot(output[seq(1,length(output[,1]),by=thin),i],type="l")

    }
  }
  dev.off()
}


### Extreme value distribution function:

rng.extreme <- function(n,a,b) {
  return(1.0/b * log(1-log(1-runif(n))/a))
}

rng.extreme.pdf <- function(x,a,b) {
  #return(a*b*exp(b*x - a*(exp(b*x)-1)))
  return(a*b*exp(a + b*x - a*exp(b*x)))
}



### Posteriors for infection times ###

infecTimes <- function(occFile) {

  occs <- read.table(occFile,colClasses='character')

  # Create output matrix

  outMat <- matrix(nrow=length(occs[,1]),ncol=length(occs[1,]))
  
  for(i in 1:length(occs[1,])) { # Go by column of occs

    unlisted <- unlist(strsplit(occs[,i],':'))

    myLabel <- unlisted[1]

    outMat[,i] <- as.numeric(unlisted[seq(2,2*length(occs[,1]),by=2)])

  }

  return(outMat)

}




######################################################################
# comparePosteriors compares the means of posterior distributions    #
######################################################################

comparePosterior <- function(postfile1,postfile2,colNum,burnIn) {

  file1=read.table(postfile1)
  file2=read.table(postfile2)

  len1=length(file1[,1])
  len2=length(file2[,1])
  
  return(mean(file1[burnIn:len1,colNum]) - mean(file2[burnIn:len2,colNum]))

}



comparePosterior <- function(postPrefix1,postPrefix2,fileNums,colNum,burnIn) {

  intervals = seq(0.1,1,by=0.1)

  outputMatrix1 = matrix(ncol=length(fileNums),nrow=length(intervals)+1)
  outputMatrix2 = matrix(ncol=length(fileNums),nrow=length(intervals)+1)

  outputCol=1
  
  for(i in fileNums) {

    # Do postPrefix1 first

    myFilename = sprintf("%s.%i.parms",postPrefix1,i)
    cat("Reading",myFilename,"...")
    data=read.table(myFilename)
    cat("Done\n")
    outputMatrix1[,outputCol] = getCDF(data[burnIn:length(data[,1]),colNum],intervals)[,2]

    # Do postPrefix2 second

    myFilename=sprintf("%s.%i.parms",postPrefix2,i)
    cat("Reading",myFilename,"...")
    data=read.table(myFilename)
    cat("Done\n")
    outputMatrix2[,outputCol] = getCDF(data[burnIn:length(data[,1]),colNum],intervals)[,2]

    outputCol = outputCol+1
  }

  return(list(a=outputMatrix1, b=outputMatrix2))
}
    


getCDF <- function(data,breaks) {

  dataSorted = sort(data)
  
  cdf = matrix(data=0,ncol=2,nrow=length(breaks)+1)
  cdf[1,1] = 0.0 # Initialise CDF at 0
  cdf[1,2] = 0.0
  cdfPos = 2 # Position in CDF
  dataPos = 1 # Position in data
  
  for(k in breaks) {

    cdf[cdfPos,1] = k
    cdf[cdfPos,2] = cdf[cdfPos-1,2] # Start at previous value

    while(dataSorted[dataPos] < k && dataPos<= length(dataSorted)) { # Count data in class
      cdf[cdfPos,2] = cdf[cdfPos,2] + 1
      dataPos = dataPos + 1
    }
    
    cdfPos = cdfPos + 1
  }

  # Divide by maximum number to get CDF
  cdf[,2] = cdf[,2]/length(dataSorted)
  
  return(cdf)
}
  



plotBeta1 <- function(filePrefix1,filePrefix2,fileNums) {
  
  for(i in fileNums) {
    
    filename1 <- sprintf("%s.%i.parms",filePrefix1,i)
    filename2 <- sprintf("%s.%i.parms",filePrefix2,i)

    file <- read.table(filename1)

    plot(density(file[30000:200000,2]),main=i)

    file <- read.table(filename2)

    lines(density(file[30000:200000,2]),col="red")

    fileout=sprintf("%s.%i.pdf",filePrefix1,i)
    dev.print(pdf,file=fileout)

  }
}



###########################################################################
# Combine posteriors in files in some kind of memory efficient way!
###########################################################################

plotPostKernDens <- function(directory) {

  myFiles <- list.files(filePath)
  
  for( i in 0:6 ) { # Seven parameters

    parmName = sprintf("beta%i",i)
    filename=sprintf("beta.%i.txt",i)
    data=scan(filename)

    graphName=sprintf("betaTrunc.%i.pdf",i)

    pdf(graphName)
    plot(density(data),main=parmName)
    dev.off()
  }

}



plotBeta1 <- function(file) {

  data=read.table(file)

  plot(data[,2],type="l")

}


      
#############################################################
# Plot parameter posteriors with priors
#############################################################

parmDens <- function(data,parmNum,priorType,lambda,nu,trueVal,xlim=NULL,ylim=NULL,bw="nrd0",...) {

  # data is a list of the 3 datasets
  # parmNum is the column in the datasets to use
  # priorType is either "gamma" or "beta"
  # lambda - prior parm1
  # nu - prior parm2

  spareArgs = list(...)

  densities <- as.list(NULL)

  # Work out plot dimensions
  reqXlim <- FALSE
  reqYlim <- FALSE

  if(priorType=="beta") {
    xlim=c(0,1)
  }
  else if(is.null(xlim)) {
    xlim <- c(0,0)
    reqXlim <- TRUE
  }

  if(is.null(ylim)) {
    ylim <- c(0,0)
    reqYlim <- TRUE
  }

  # Calculate densities
  for(dataSet in 1:length(data)) {
    densities <- c(densities,list(density(data[[dataSet]][,parmNum],bw=bw)))

    # Set limits
    if(reqXlim) {
      xMax <- max(densities[[dataSet]]$x)
      if(xMax > xlim[2]) xlim[2] <- xMax
    }
    if(reqYlim) {
      yMax <- max(densities[[dataSet]]$y)
      if(yMax > ylim[2]) ylim[2] <- yMax
    }
  }

  # Calculate prior
  x <- seq(xlim[1],xlim[2],len=1000)
  if(priorType == "beta") {
    y <- dbeta(x,lambda,nu)
  }
  else if(priorType == "gamma") {
    y <- dgamma(x,lambda,nu)
  }
  else {
    stop("Invalid prior type")
  }

  # Start the plot
  if(!is.null(spareArgs$cex)) par(cex=spareArgs$cex)
  if(!is.null(spareArgs$lwd)) par(lwd=spareArgs$lwd)

  xlab = spareArgs$xlab # NULL if not specified - poss dangerous?
  plot(x,y,type="l",xlim=xlim,ylim=ylim,xlab=xlab,ylab="Density")
  colours=c("red","green","blue")
  for(i in 1:length(data)) {
    lines(densities[[i]]$x,densities[[i]]$y,col=colours[i])
  }
  abline(v=trueVal,lty=2,col="grey")
  
  # Print smoothing bandwidths
  for(dens in densities) {
    cat("BW: ", dens$bw, "\n")
  }

}
