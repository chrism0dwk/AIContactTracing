# ./plotipt.r
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

#
###########################################################################
#    Copyright (C) 2009 by Chris Jewell                                    #
#    chris.jewell@warwick.ac.uk                                            #
#                                                                          #
#    This program is free software; you can redistribute it and/or modify  #
#    it under the terms of the GNU General Public License as published by  #
#    the Free Software Foundation; either version 2 of the License, or     #
#    (at your option) any later version.                                   #
#                                                                          #
#    This program is distributed in the hope that it will be useful,       #
#    but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#    GNU General Public License for more details.                          #
#                                                                          #
#    You should have received a copy of the GNU General Public License     #
#    along with this program; if not, write to the                         #
#    Free Software Foundation, Inc.,                                       #
#    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
############################################################################


plotEpiIpt <- function(iptFilename, nTotal,sep=" ",skip=0) {

   iptFile = read.table(iptFilename,sep=sep,skip=skip)
   numInfecs = length(iptFile[,1])
   
   eventList = data.frame(time = numeric(numInfecs*3), label = numeric(numInfecs*3), type = character(numInfecs*3), stringsAsFactors = FALSE)
   popSize = data.frame(S = numeric(numInfecs*3), I = numeric(numInfecs*3), N = numeric(numInfecs*3), R = numeric(numInfecs*3))

   
   for(i in 1:numInfecs) {

     # I
     elRow = (i-1)*3 + 1
     eventList$time[elRow] = iptFile[i,2]
     eventList$label[elRow] = iptFile[i,1]
     eventList$type[elRow] = 'i'
     
     # N
     elRow = (i-1)*3 + 2
     eventList$time[elRow] = iptFile[i,3]
     eventList$label[elRow] = iptFile[i,1]
     eventList$type[elRow] = 'n'

     # R
     elRow = (i-1)*3 + 3
     eventList$time[elRow] = iptFile[i,4]
     eventList$label[elRow] = iptFile[i,1]
     eventList$type[elRow] = 'r'

   }

   # Sort eventList
   eventList <- eventList[order(eventList$time),]

   # Calculate popn sizes
   S = nTotal; I = 0; N = 0; R = 0

   for(i in 1:length(eventList$time)) {
     if (eventList$type[i] == 'i') {
       S <- S - 1
       I <- I + 1
       popSize$S[i] = S
       popSize$I[i] = I
       popSize$N[i] = N
       popSize$R[i] = R
     }
     else if (eventList$type[i] == 'n') {
       I <- I - 1
       N <- N + 1
       popSize$S[i] = S
       popSize$I[i] = I
       popSize$N[i] = N
       popSize$R[i] = R
     }
     else if (eventList$type[i] == 'r') {
       N <- N - 1
       R <- R + 1
       popSize$S[i] = S
       popSize$I[i] = I
       popSize$N[i] = N
       popSize$R[i] = R
     }
     else {
       cat("Unknown event!\n")
     }
   }


   # Plot graph of I and N

   plot(eventList$time, popSize$I, col="red",type="l")
   lines(eventList$time, popSize$N, col="magenta")

   return(data.frame(eventList,popSize))
}
		
