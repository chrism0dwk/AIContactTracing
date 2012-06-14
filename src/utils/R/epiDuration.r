# ./epiDuration.r
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

# epiDuration takes a bunch of .ipt files and plots a
# histogram of epidemic durations

epiDuration <- function(dir,filename) {


  myFiles <- dir(path=dir,pattern=filename)
  
  durs <- NULL
  
  for (file in myFiles) {
    myData <- read.table(paste(dir,"/",file,sep=""))
    durs <- c(durs,max(myData[,4]))
  }

  hist(durs)

  return(durs)
}
