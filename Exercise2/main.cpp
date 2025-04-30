#include <iostream>
#include "PolygonalMesh.hpp"
#include "Utils.hpp"
#include "ExportParaview/UCDUtilities.hpp"
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;
using namespace PolygonalLibrary;

bool edges_test(PolygonalMesh& mesh);
bool areas_test(PolygonalMesh& mesh);

int main()
{
    PolygonalMesh mesh;

    if(!ImportMesh(mesh))
    {
        cerr << "file not found" << endl;
        return 1;
    }

    /// Per visualizzare online le mesh:
    /// 1. Convertire i file .inp in file .vtu con https://meshconverter.it/it
    /// 2. Caricare il file .vtu su https://kitware.github.io/glance/app/

    Gedim::UCDUtilities utilities;
    {
        vector<Gedim::UCDProperty<double>> cell0Ds_properties(1);

        cell0Ds_properties[0].Label = "Marker";
        cell0Ds_properties[0].UnitLabel = "-";
        cell0Ds_properties[0].NumComponents = 1;

        vector<double> cell0Ds_marker(mesh.NumCell0Ds, 0.0);
        for(const auto &m : mesh.MarkerCell0Ds)
            for(const unsigned int id: m.second)
                cell0Ds_marker.at(id) = m.first;

        cell0Ds_properties[0].Data = cell0Ds_marker.data();

        utilities.ExportPoints("./Cell0Ds.inp",
                               mesh.Cell0DsCoordinates,
                               cell0Ds_properties);
    }

    {

        vector<Gedim::UCDProperty<double>> cell1Ds_properties(1);

        cell1Ds_properties[0].Label = "Marker";
        cell1Ds_properties[0].UnitLabel = "-";
        cell1Ds_properties[0].NumComponents = 1;

        vector<double> cell1Ds_marker(mesh.NumCell1Ds, 0.0);
        for(const auto &m : mesh.MarkerCell1Ds)
            for(const unsigned int id: m.second)
                cell1Ds_marker.at(id) = m.first;

        cell1Ds_properties[0].Data = cell1Ds_marker.data();

        utilities.ExportSegments("./Cell1Ds.inp",
                                 mesh.Cell0DsCoordinates,
                                 mesh.Cell1DsExtrema,
                                 {},
                                 cell1Ds_properties);
    }
	
    edges_test(mesh);
    areas_test(mesh);   

    return 0;
}

bool edges_test(PolygonalMesh& mesh)
{
	for(unsigned int i = 0; i<mesh.NumCell2Ds; i++)
	{
		for(unsigned int j = 0; j<mesh.Cell2DsEdges[i].size();j++)
		{				
			vector<unsigned int>& edges = mesh.Cell2DsEdges[i];
			
			int& Origin = mesh.Cell1DsExtrema(0,edges[j]);
			int& End = mesh.Cell1DsExtrema(1,edges[j]);
			
			double& X_Origin = mesh.Cell0DsCoordinates(0,Origin);
			double& Y_Origin = mesh.Cell0DsCoordinates(1,Origin);
			double& X_End = mesh.Cell0DsCoordinates(0,End_index);
			double& Y_End = mesh.Cell0DsCoordinates(1,End_index);	
			
			double distance = sqrt(pow(X_Origin-X_End,2)+pow(Y_Origin-Y_End,2));
			
			if(distance < 1e-16)
			{
				cout<< "Poligono " << i << " ha problema sui segmenti" <<endl;
				return false;
			}
		}
	}
	cout<<"Tutti i segmenti ok"<<endl;
	return true;
}

bool areas_test(PolygonalMesh& mesh)
{	
	for(unsigned int i = 0; i<mesh.NumCell2Ds; i++)
	{
		double area = 0.0;
		unsigned int n = mesh.Cell2DsVertices[i].size();
		for(unsigned int j = 0; j < n; j++)
		{
			unsigned int& P1 = mesh.Cell2DsVertices[i][j];
			unsigned int& P2 = mesh.Cell2DsVertices[i][(j+1)%n];
			
			double& X_P1 = mesh.Cell0DsCoordinates(0,P1);
			double& Y_P1 = mesh.Cell0DsCoordinates(1,P1);
			double& X_P2 = mesh.Cell0DsCoordinates(0,P2);
			double& Y_P2 = mesh.Cell0DsCoordinates(1,P2);
			
			area += X_P1*Y_P2-X_P2*Y_P1;
		}		
		area = 0.5*abs(area);
		
		if(area < 1e-16)
		{
			cout<< "Poligono " << i << " ha area nulla"<<endl;
			return false;
		}
	}
	cout<<"Tutte le aree sono diverse da zero"<<endl;
	return true;
}
