# ./riskAnalysis.r
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

# R script to process simulation output files #

infecFreq <- function(n.farms,n.sims) {

  infecs <- rep(0,n.farms);
  
  for(i in 1:n.sims) {  # Iterate over the files
     simdata <- read.table(paste("fwd_sim",i,".txt",sep=""))
     for(farm in 1:n.farms) { # Iterate over the farms
       if(any(simdata[,2]==farm)) {
         infecs[farm] = infecs[farm] + 1
       }
     }
   }
  
  return(infecs)  # Divide by the number of simulations
}


risk.col <- function(infecs,locs,n.cols) {
#  require(britIsles)
  require(splancs)
  infec.order <- order(infecs)
  locs <- locs[infec.order,]
  infecs <- infecs[infec.order]
  n.farms <- length(infecs)
  colLevels <- rainbow(n.cols,start=0,end=4/6)
  colLevels <- rev(colLevels)
  farm.cols <- numeric(n.farms)
  
  # Now we assign a colour from colLevels to each farm

  #breaks <- seq(min(infecs),max(infecs),length=n.cols+1)
  breaks <- 0:(n.cols+1)
print(breaks)
  for(i in 1:n.farms) {

    if(infecs[i] <= breaks[length(breaks)]) {
      for(j in 1:n.cols) {
        
        if(infecs[i] <= breaks[j+1]) {
          farm.cols[i] = colLevels[j]
          break
        }
        else farm.cols[i] = colLevels[length(colLevels)]
      }
    }
    else farm.cols[i] = colLevels[length(colLevels)]
    
  }

  britIsles(scale=0.01)
  points(locs,pch=20,col=farm.cols)

  breaks = round(breaks[-1],digits=5)
  breaks = breaks[-length(breaks)]
  breaks.text <- as.character(breaks)
  breaks.text[1] <- paste("<",breaks[1])
  for(i in 2:length(breaks)) {
    breaks.text[i] = paste("-",breaks[i])
  }
  
  legend(-4e06,1e7,breaks.text,fill=colLevels)
  

  return(farm.cols)
}


region.risk <- function(infecs,locs,n.cols,graph=FALSE) {
  require(britIsles)
  num.regions <- length(biData$Name)
  region.incidence <- rep(0,num.regions)

  for(i in 1:num.regions) {
    region.farms <- inpip(locs,biCoordsList[[i]]/0.1)
    if(length(region.farms) > 0) {
      for(j in 1:length(region.farms)) {
        region.incidence[i] = region.incidence[i] + infecs[region.farms[j]]
      }
    }
    else {
      region.incidence[i] = 0.000001
    }    
  }

  region.incidence = region.incidence / 1000
#  region.incidence = log(region.incidence)

  if(graph==TRUE) {
                                        # Now set up a colours vector for the regions
    
    colLevels <- rainbow(n.cols-1,start=0,end=4/6)
    colLevels <- rev(colLevels)
    region.cols <- numeric(num.regions)
    
    breaks <- seq(min(region.incidence),max(region.incidence),length=n.cols)
    
    for(i in 1:num.regions) {
      for(j in 2:n.cols) {
         if(region.incidence[i] <= breaks[j]) {
          region.cols[i] = colLevels[j-1]
          break
        }
      }
    }
    
    britIsles(scale=0.1)
    
    for(i in 1:num.regions) {
      polymap(biCoordsList[[i]]/0.1,col=region.cols[i],add=T)
    }

    breaks = round(breaks[-1],digits=3)
    breaks.text <- as.character(breaks)
    breaks.text[1] <- paste("<",breaks[1])
    for(i in 2:length(breaks)) {
      breaks.text[i] = paste(breaks[i-1],"-",breaks[i])
    }

    points(locs,pch=".",cex=1,col="white")
    legend(-4e05,1e6,breaks.text,fill=colLevels)

         }

  
  return(region.incidence)

}



kernelsmooth <- function(I1,infecs,locs,h,locPlot=TRUE) {
  require(splancs)
  require(britIsles)
  I1coords <- locs[I1,]
  locs <- locs[-I1,]
  infecs <- infecs[-I1]
  num.farms <- length(locs[,1])
  num.infecs <- sum(infecs)
  cases <- matrix(ncol=2,nrow=num.infecs)
  case <- 1
  cat("dim cases =",dim(cases),"\n")
  
  for(i in 1:num.farms) {
    if(infecs[i] > 0) {
      for(j in 1:infecs[i]) {
        cases[case,1] <- locs[i,1]
        cases[case,2] <- locs[i,2]
        case <- case + 1
      }
    }
  }

  poly <- matrix(nrow=4,ncol=2)
  poly[1,1] = -2e05; poly[1,2] = 0
  poly[2,1] = -2e05; poly[2,2] = 1250000
  poly[3,1] = 800000; poly[3,2] = 1250000
  poly[4,1] = 800000; poly[4,2] = 0

  smooth <- kernel2d(as.points(cases),poly,h,nx=200,ny=200)
  #britIsles(scale=0.1,col="white")
  plot(0,0,type="n",xlim=c(-2e05,7.5e05),ylim=c(0,1205000),axes=F,xlab="",ylab="")
  image(smooth,add=T,col=rev(rainbow(9,start=0,end=4/6)))
  britIsles(scale=0.1,add=T)
  points(I1coords,col="white")
  if(locPlot==TRUE) points(locs,pch=20,cex=0.1,col="white")
  breaks <- seq(min(infecs/1000),max(infecs/1000),length=10)
  breaks = round(breaks[-1],digits=2)
  breaks.text <- as.character(breaks)
  breaks.text[1] <- paste("<",breaks[1])
  for(i in 2:length(breaks)) {
    breaks.text[i] = paste("-",breaks[i])
  }
  
  legend(-1.8e05,1.2e6,breaks.text,fill=rev(rainbow(9,start=0,end=4/6)),bg="white")


  return(smooth)
}
