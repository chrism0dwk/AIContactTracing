# ./r_anim.r
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

# This file creates an animation out of simulation data

simAnimdb <- function(simFile) {
  require(britIsles,lib.loc="~/storage/RLibs")
  require(Rdbi)
  require(Rdbi.PgSQL)

  database <- dbConnect(PgSQL(),host="localhost",port=5432,dbname="gbpr",user="jewellc",password="lums2005")
  data <- read.table(simFile)
  coords <- dbGetQuery(database,"SELECT easting,northing FROM gbpr_new WHERE broiler = TRUE OR layer = TRUE ORDER BY repository_id")
  #results = matrix(ncol=4,nrow=length(coords[,1]))
  #names(results) <- c("Farm","I","N","R")
  #results[,1] = seq(0,length(results[,1])-1)
  britIsles(scale=0.1)
  
  for(i in 1:length(data[,1])) {

    if (data[i,3] == 'i') {
      points(coords[data[i,2]+1,1],coords[data[i,2]+1,2],pch=".",col="red")
    }
    else if (data[i,3] == 'n') {
      points(coords[data[i,2]+1,1],coords[data[i,2]+1,2],pch=".",col="green")
    }
    else if (data[i,3] == 'r') {
      points(coords[data[i,2]+1,1],coords[data[i,2]+1,2],pch=".",col="blue")
    }
  }
}

simAnim <- function(iptFile,covars,time.step=1) {

  data <- read.csv(iptFile)
  covars <- read.csv(covars)
  coords <- data.frame(cph=covars$cph,x=covars$easting,y=covars$northing)
  
  plot(0,0,xlim=c(min(covars$easting),max(covars$easting)),ylim=c(min(covars$northing),max(covars$northing)),type="n")

  points(coords$x,coords$y,col="grey",pch="x",cex=0.5)
  #legend(-3e05,1e6,pch="x",pt.cex=0.5,legend=c("S","I","N","R"),col=c("grey","red","green","blue"))

  time = 0
  maxtime = max(data$I,data$N,data$R)
  
  while (time <= maxtime) {
    cat("Time =",time,"\n")
	infected = coords[match(data$id[time <= data$I & data$I < time + time.step],coords$cph),]
	notified = coords[match(data$id[time <= data$N & data$N < time + time.step],coords$cph),]
	removed = coords[match(data$id[time <= data$R & data$R < time + time.step],coords$cph),]
	
    points(infected$x,infected$y,pch="x",cex=0.5,col="red")
    points(notified$x,notified$y,pch="x",cex=0.5,col="magenta")
    points(removed$x,removed$y,pch="x",cex=0.5,col="green")
    
    Sys.sleep(1)
    time = time + time.step
    
  }

}
