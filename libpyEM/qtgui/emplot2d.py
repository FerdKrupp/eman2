#!/usr/bin/env python

#
# Author: Steven Ludtke, 04/10/2003 (sludtke@bcm.edu)
# Copyright (c) 2000-2006 Baylor College of Medicine
#
# This software is issued under a joint BSD/GNU license. You may use the
# source code in this file under either license. However, note that the
# complete EMAN2 and SPARX software packages have some GPL dependencies,
# so you are responsible for compliance with the licenses of these packages
# if you opt to use BSD licensing. The warranty disclaimer below holds
# in either instance.
#
# This complete copyright notice must be included in any revised version of the
# source code. Additional authorship citations may be added, but existing
# author citations must be preserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston MA 02111-1307 USA
#
#

import PyQt4
from PyQt4 import QtCore, QtGui, QtOpenGL
from PyQt4.QtCore import Qt
from OpenGL import GL,GLU,GLUT
from valslider import ValSlider
from math import *
from EMAN2 import *
import EMAN2
import sys
import numpy
from emimageutil import ImgHistogram
from weakref import WeakKeyDictionary
from pickle import dumps,loads
import struct

import matplotlib
from matplotlib.backends.backend_agg import FigureCanvasAgg
from matplotlib.figure import Figure
#matplotlib.use('Agg')

linetypes=["-","--",":","-."]
symtypes=["o","s","+","2","1"]
colortypes=["k","b","r","g"]


class EMPlot2D(QtOpenGL.QGLWidget):
	"""A QT widget for drawing 2-D plots using matplotlib
	"""
	def __init__(self, parent=None):
		fmt=QtOpenGL.QGLFormat()
		fmt.setDoubleBuffer(True);
		QtOpenGL.QGLWidget.__init__(self,fmt, parent)
		self.axes={}
		self.pparm={}
		self.inspector=None

		self.data={}				# List of Lists to plot 
		
	def setData(self,key,data):
		"""Set a keyed data set. The key should generally be a string describing the data.
		'data' is a tuple/list of tuples/list representing all values for a particular
		axis. eg - the points: 1,5; 2,7; 3,9 would be represented as ((1,2,3),(5,7,9)).
		Multiple axes may be set, and which axis represents which axis in the plot can be
		selected by the user."""
		
		try:
			if len(data)>1 : self.axes[key]=(0,1,-1)
			else : self.axes[key]=(-1,0,-1)
		except: return
		
		self.pparm[key]=(0,1,0,1,0,0,5)
		
		if data : self.data[key]=data
		else : del self.data[key]
		
		
		if self.inspector: self.inspector.datachange()
		
		self.updateGL()
		
	def initializeGL(self):
		GL.glClearColor(0,0,0,0)
		
	def paintGL(self):
		GL.glClear(GL.GL_COLOR_BUFFER_BIT | GL.GL_DEPTH_BUFFER_BIT)
#		GL.glLoadIdentity()
#		GL.glTranslated(0.0, 0.0, -10.0)
		
		if not self.data : return
		
		fig=Figure((self.width()/72.0,self.height()/72.0),dpi=72.0)
		ax=fig.add_axes((.1,.05,.9,.9))
		canvas=FigureCanvasAgg(fig)
		
		for i in self.axes.keys():
			j=self.axes[i]
			if j[0]==-1 : x=range(len(self.data[i][0]))
			else : x=self.data[i][self.axes[i][0]]
			if j[1]==-1 : y=range(len(self.data[i][0]))
			else : y=self.data[i][self.axes[i][1]]
			parm=""
			parm+=colortypes[self.pparm[i][0]]
			if self.pparm[i][1]: 
				parm+=linetypes[self.pparm[i][2]]
			if self.pparm[i][4]:
				parm+=symtypes[self.pparm[i][5]]
				
			ax.plot(x,y,parm,linewidth=self.pparm[i][3],markersize=self.pparm[i][6])
		
		canvas.draw()
		a = canvas.tostring_rgb()  # save this and convert to bitmap as needed
		
		GL.glRasterPos(0,self.height()-1)
		GL.glPixelZoom(1.0,-1.0)
#		print "paint ",self.width(),self.height(), self.width()*self.height(),len(a)
		GL.glPixelStorei(GL.GL_UNPACK_ALIGNMENT,1)
		GL.glDrawPixels(self.width(),self.height(),GL.GL_RGB,GL.GL_UNSIGNED_BYTE,a)

	def resizeGL(self, width, height):
#		print "resize ",self.width()
		side = min(width, height)
		GL.glViewport(0,0,self.width(),self.height())
	
		GL.glMatrixMode(GL.GL_PROJECTION)
		GL.glLoadIdentity()
		GLU.gluOrtho2D(0.0,self.width(),0.0,self.height())
		GL.glMatrixMode(GL.GL_MODELVIEW)
		GL.glLoadIdentity()
		
	
	def showInspector(self,force=0):
		if not force and self.inspector==None : return
		
		if not self.inspector : self.inspector=EMPlot2DInspector(self)
		self.inspector.show()
		self.inspector.datachange()
	
	def closeEvent(self,event) :
		if self.inspector: self.inspector.close()
	
	def setAxes(self,key,xa,ya,za):
		if self.axes[key]==(xa,ya,za) : return
		self.axes[key]=(xa,ya,za)
		self.updateGL()
		
	def setPlotParms(self,key,color,line,linetype,linewidth,sym,symtype,symsize):
		self.pparm[key]=(color,line,linetype,linewidth,sym,symtype,symsize)
		self.updateGL()
	
	#def dragEnterEvent(self,event):
		
		#if event.provides("application/x-eman"):
			#event.setDropAction(Qt.CopyAction)
			#event.accept()

	
	#def dropEvent(self,event):
#=		if EMAN2.GUIbeingdragged:
			#self.setData(EMAN2.GUIbeingdragged)
			#EMAN2.GUIbeingdragged=None
		#elif event.provides("application/x-eman"):
			#x=loads(event.mimeData().data("application/x-eman"))
			#self.setData(x)
			#event.acceptProposedAction()

	
	def mousePressEvent(self, event):
		if event.button()==Qt.MidButton:
			self.showInspector(1)
		#elif event.button()==Qt.RightButton:
			#self.rmousedrag=(event.x(),event.y())
		#elif event.button()==Qt.LeftButton:
			#if self.mmode==0:
				#self.emit(QtCore.SIGNAL("mousedown"), event)
				#return
			#elif self.mmode==1 :
				#try: 
					#del self.shapes["MEASL"]
				#except: pass
				#self.addShape("MEAS",("line",.5,.1,.5,lc[0],lc[1],lc[0]+1,lc[1],2))
	
	def mouseMoveEvent(self, event):
		pass
		#if self.rmousedrag and event.buttons()&Qt.RightButton:
			#self.origin=(self.origin[0]+self.rmousedrag[0]-event.x(),self.origin[1]-self.rmousedrag[1]+event.y())
			#self.rmousedrag=(event.x(),event.y())
			#self.update()
		#elif event.buttons()&Qt.LeftButton:
			#if self.mmode==0:
				#self.emit(QtCore.SIGNAL("mousedrag"), event)
				#return
			#elif self.mmode==1 :
				#self.addShape("MEAS",("line",.5,.1,.5,self.shapes["MEAS"][4],self.shapes["MEAS"][5],lc[0],lc[1],2))
				#dx=lc[0]-self.shapes["MEAS"][4]
				#dy=lc[1]-self.shapes["MEAS"][5]
				#self.addShape("MEASL",("label",.5,.1,.5,lc[0]+2,lc[1]+2,"%d,%d - %d,%d\n%1.1f,%1.1f (%1.2f)"%(self.shapes["MEAS"][4],self.shapes["MEAS"][5],lc[0],lc[1],dx,dy,hypot(dx,dy)),10,-1))
	
	def mouseReleaseEvent(self, event):
		pass
		#lc=self.scrtoimg((event.x(),event.y()))
		#if event.button()==Qt.RightButton:
			#self.rmousedrag=None
		#elif event.button()==Qt.LeftButton:
			#if self.mmode==0:
				#self.emit(QtCore.SIGNAL("mouseup"), event)
				#return
			#elif self.mmode==1 :
				#self.addShape("MEAS",("line",.5,.1,.5,self.shapes["MEAS"][4],self.shapes["MEAS"][5],lc[0],lc[1],2))

	def wheelEvent(self, event):
		pass
		#if event.delta() > 0:
			#self.setScale(self.scale + MAG_INC)	
		#elif event.delta() < 0:
			#if ( self.scale - MAG_INC > 0 ):
				#self.setScale(self.scale - MAG_INC)
		## The self.scale variable is updated now, so just update with that
		#if self.inspector: self.inspector.setScale(self.scale)

class EMPlot2DInspector(QtGui.QWidget):
	def __init__(self,target) :
		QtGui.QWidget.__init__(self,None)
		self.target=target
		
		self.hbl = QtGui.QHBoxLayout(self)
		self.hbl.setMargin(0)
		self.hbl.setSpacing(6)
		self.hbl.setObjectName("hbl")
		
		# plot list
		self.setlist=QtGui.QListWidget(self)
		self.setlist.setSizePolicy(QtGui.QSizePolicy.Preferred,QtGui.QSizePolicy.Expanding)
		self.hbl.addWidget(self.setlist)
		
		self.vbl = QtGui.QVBoxLayout()
		self.vbl.setMargin(0)
		self.vbl.setSpacing(6)
		self.vbl.setObjectName("vbl")
		self.hbl.addLayout(self.vbl)
		
		self.color=QtGui.QComboBox(self)
		self.color.addItem("black")
		self.color.addItem("blue")
		self.color.addItem("red")
		self.color.addItem("green")
		self.vbl.addWidget(self.color)

		self.hbl2 = QtGui.QHBoxLayout()
		self.hbl2.setMargin(0)
		self.hbl2.setSpacing(6)
		self.vbl.addLayout(self.hbl2)
				
		# This is for line parms
		self.vbl2b = QtGui.QVBoxLayout()
		self.vbl2b.setMargin(0)
		self.vbl2b.setSpacing(6)
		self.hbl2.addLayout(self.vbl2b)
				
		self.lintog=QtGui.QPushButton(self)
		self.lintog.setText("Line")
		self.lintog.setCheckable(1)
		self.vbl2b.addWidget(self.lintog)
				
		self.linsel=QtGui.QComboBox(self)
		self.linsel.addItem("------")
		self.linsel.addItem("- - - -")
		self.linsel.addItem(".......")
		self.linsel.addItem("-.-.-.-")
		self.vbl2b.addWidget(self.linsel)
		
		self.linwid=QtGui.QSpinBox(self)
		self.linwid.setRange(1,10)
		self.vbl2b.addWidget(self.linwid)
		
		# This is for point parms
		self.vbl2a = QtGui.QVBoxLayout()
		self.vbl2a.setMargin(0)
		self.vbl2a.setSpacing(6)
		self.hbl2.addLayout(self.vbl2a)
				
		self.symtog=QtGui.QPushButton(self)
		self.symtog.setText("Symbol")
		self.symtog.setCheckable(1)
		self.vbl2a.addWidget(self.symtog)

		self.symsel=QtGui.QComboBox(self)
		self.symsel.addItem("circle")
		self.symsel.addItem("square")
		self.symsel.addItem("plus")
		self.symsel.addItem("triup")
		self.symsel.addItem("tridown")
		self.vbl2a.addWidget(self.symsel)
		
		self.symsize=QtGui.QSpinBox(self)
		self.symsize.setRange(0,25)
		self.vbl2a.addWidget(self.symsize)
		
		# per plot column selectors
		self.slidex=QtGui.QSpinBox(self)
		self.slidex.setRange(-1,1)
		self.vbl.addWidget(self.slidex)
		#self.slidex=ValSlider(self,(-1,1),"X col:",0)
		#self.slidex.setIntonly(1)
		#self.vbl.addWidget(self.slidex)
		
		self.slidey=QtGui.QSpinBox(self)
		self.slidey.setRange(-1,1)
		self.vbl.addWidget(self.slidey)
		#self.slidey=ValSlider(self,(-1,1),"Y col:",1)
		#self.slidey.setIntonly(1)
		#self.vbl.addWidget(self.slidey)
		
		self.slidec=QtGui.QSpinBox(self)
		self.slidec.setRange(-1,1)
		self.vbl.addWidget(self.slidec)
		#self.slidec=ValSlider(self,(-1,1),"C col:",-1)
		#self.slidec.setIntonly(1)
		#self.vbl.addWidget(self.slidec)
		
		self.setLayout(self.hbl)

		self.quiet=0
		
		QtCore.QObject.connect(self.slidex, QtCore.SIGNAL("valueChanged(int)"), self.newCols)
		QtCore.QObject.connect(self.slidey, QtCore.SIGNAL("valueChanged(int)"), self.newCols)
		QtCore.QObject.connect(self.slidec, QtCore.SIGNAL("valueChanged(int)"), self.newCols)
		QtCore.QObject.connect(self.setlist,QtCore.SIGNAL("currentRowChanged(int)"),self.newSet)
		QtCore.QObject.connect(self.color,QtCore.SIGNAL("currentIndexChanged(QString)"),self.updPlot)
		QtCore.QObject.connect(self.symtog,QtCore.SIGNAL("clicked()"),self.updPlot)
		QtCore.QObject.connect(self.symsel,QtCore.SIGNAL("currentIndexChanged(QString)"),self.updPlot)
		QtCore.QObject.connect(self.symsize,QtCore.SIGNAL("valueChanged(int)"),self.updPlot)
		QtCore.QObject.connect(self.lintog,QtCore.SIGNAL("clicked()"),self.updPlot)
		QtCore.QObject.connect(self.linsel,QtCore.SIGNAL("currentIndexChanged(QString)"),self.updPlot)
		QtCore.QObject.connect(self.linwid,QtCore.SIGNAL("valueChanged(int)"),self.updPlot)
		self.datachange()
		
		
		#QtCore.QObject.connect(self.gammas, QtCore.SIGNAL("valueChanged"), self.newGamma)
		#QtCore.QObject.connect(self.invtog, QtCore.SIGNAL("toggled(bool)"), target.setInvert)
		#QtCore.QObject.connect(self.mmode, QtCore.SIGNAL("buttonClicked(int)"), target.setMMode)

	def updPlot(self,s=None):
		if self.quiet : return
		self.target.setPlotParms(str(self.setlist.currentItem().text()),self.color.currentIndex(),self.lintog.isChecked(),
			self.linsel.currentIndex(),self.linwid.value(),self.symtog.isChecked(),
			self.symsel.currentIndex(),self.symsize.value())

	def newSet(self,row):
		self.quiet=1
		i=str(self.setlist.item(row).text())
		self.slidex.setRange(-1.5,len(self.target.data[i])-1)
		self.slidey.setRange(-1.5,len(self.target.data[i])-1)
		self.slidec.setRange(-1.5,len(self.target.data[i])-1)
		self.slidex.setValue(self.target.axes[i][0])
		self.slidey.setValue(self.target.axes[i][1])
		self.slidec.setValue(self.target.axes[i][2])
		
		pp=self.target.pparm[i]
		self.color.setCurrentIndex(pp[0])
		
		self.lintog.setChecked(pp[1])
		self.linsel.setCurrentIndex(pp[2])
		self.linwid.setValue(pp[3])
		
		self.symtog.setChecked(pp[4])
		self.symsel.setCurrentIndex(pp[5])
		self.symsize.setValue(pp[6])
		self.quiet=0

	def newCols(self,val):
		if self.target: 
			self.target.setAxes(str(self.setlist.currentItem().text()),self.slidex.value,self.slidey.value,self.slidec.value)
	
	def datachange(self):
		
		self.setlist.clear()
		
		keys=self.target.data.keys()
		keys.sort()
		
		for i,j in enumerate(keys) :
			self.setlist.addItem(j)
		
		
# This is just for testing, of course
if __name__ == '__main__':
	app = QtGui.QApplication(sys.argv)
	window = EMPlot2D()
	if len(sys.argv)==1 : 
		l=[i/30.*pi for i in range(30)]
		window.setData("test",[[1,2,3,4],[2,3,4,3],[3,4,5,2]])
		window.setData("test2",[l,[sin(i) for i in l],[cos(i) for i in l]])
	else:
		try:
			# if it's an image file
			im=EMData(sys.argv[1])
			# This could be faster...
			data=[[im.get_value_at(i,j) for j in range(im.get_ysize())] for j in range(im.get_xsize())]
		except:
			try:
				# Maybe a .fp file
				fin=file(sys.argv[1])
				fph=struct.unpack("120sII",fin.read(128))
				ny=fph[1]
				nx=fph[2]
				data=[]
				for i in range(nx):
					data.append(struct.unpack("%df"%ny,fin.read(4*ny)))
			except:
				# Probably a text file
				fin=file(sys.argv[1])
				fin.seek(0)
				rdata=fin.readlines()
				rdata=[i for i in rdata if i[0]!='#']
				if ',' in rdata[0]: rdata=[[float(j) for j in i.split(',')] for i in rdata]
				else : rdata=[[float(j) for j in i.split()] for i in rdata]
				nx=len(rdata[0])
				ny=len(rdata)
				
				print nx,ny
				data=[[rdata[j][i] for j in range(ny)] for i in range(nx)]
				
		window.setData(sys.argv[1].split("/")[-1],data)
			
				

	window.show()
		
	sys.exit(app.exec_())
