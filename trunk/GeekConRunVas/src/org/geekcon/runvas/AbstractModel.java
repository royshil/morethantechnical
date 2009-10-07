package org.geekcon.runvas;

import java.util.ArrayList;

import org.geekcon.runvas.utils.Face;
import org.geekcon.runvas.utils.Vector3D;
import org.geekcon.runvas.utils.Vertex;

public abstract class AbstractModel {
        protected Vector3D location;
        protected Vector3D globalOrientation;   //angles in all directions
        protected Vector3D localOrientation;
        protected Vector3D color;                       //RGB color
//      private ArrayList<Face> faces; CBE
        private ArrayList<Face> triFaces;
        private ArrayList<Face> quadFaces;
        private ArrayList<Vertex> vertices;
        protected float scale;
        
        @Override
        public AbstractModel clone() throws CloneNotSupportedException {
                return (AbstractModel)this.clone();
        }
        
        public AbstractModel() {
//              faces = new ArrayList<Face>(); CBE
                triFaces = new ArrayList<Face>();
                quadFaces = new ArrayList<Face>();
                vertices = new ArrayList<Vertex>();
                location = Vector3D.origin;
                globalOrientation = Vector3D.origin;
                localOrientation = Vector3D.origin;
                color = Vector3D.ones;
                scale = 1f;
        }

        public Vector3D getLocation() {
                return location;
        }

        public void setLocation(Vector3D location) {
                this.location = location;
        }

        public Vector3D getGlobalOrientation() {
                return globalOrientation;
        }

        public void setGlobalOrientation(Vector3D globalOrientation) {
                this.globalOrientation = globalOrientation;
        }

        public Vector3D getColor() {
                return color;
        }

        public void setColor(Vector3D color) {
                this.color = color;
        }

//      public ArrayList<Face> getFaces() { CBE
//              return faces;
//      }
//
//      public void setFaces(ArrayList<Face> faces) {
//              this.faces = faces;
//      }
        
        public ArrayList<Face> getTriFaces() { 
                return triFaces;
        }
        
        public void setTriFaces(ArrayList<Face> faces) {
                this.triFaces = faces;
        }
        
        public ArrayList<Face> getQuadFaces() { 
                return quadFaces;
        }
        
        public void setQuadFaces(ArrayList<Face> faces) {
                this.quadFaces = faces;
        }

        public ArrayList<Vertex> getVertices() {
                return vertices;
        }

        public void setVertices(ArrayList<Vertex> vertices) {
                this.vertices = vertices;
        }
        public float getScale() {
                return scale;
        }

        public void setScale(float scale) {
                this.scale = scale;
        }

        public Vector3D getLocalOrientation() {
                return localOrientation;
        }

        public void setLocalOrientation(Vector3D localOrientation) {
                this.localOrientation = localOrientation;
        }
}
