#include <stdio.h>
#include <memory>
#include <map>
#include <new>

class Feature
{
public:
    Feature() {
	fillDataFactory[eCircleStorage] = std::make_unique<Factory<Circle> >();
	fillDataFactory[ePolygoneStorage] = std::make_unique<Factory<Polygone> >();
    }

    enum FeatureType {eUnknown, eCircle, eTriangle, eSquare}; 
    enum StorageType { eCircleStorage, ePolygoneStorage};

    StorageType convertFeatureToStorageType(FeatureType aFeatureType) { return aFeatureType == eCircle ? eCircleStorage : ePolygoneStorage;}

    bool isValid() const {
        return figure ? true : false;
    } 

    bool read(FILE* file)
    {   
        FeatureType type;
        if (fread(&type, sizeof(FeatureType), 1, file) != sizeof(FeatureType)) {
            return false;
            //it was unclear for me if the eUknown should invalidate the figure that was already
            //loaded in the class. I assumed that it shouldnt.
	}
        auto it = figure_sizes.find(type);
        if(it == figure_sizes.end()){
            return false;	
	}
        try {	
            Tpoints_ptr points(new double[it->second]);
            if( fread(points.get(), sizeof(double), it->second, file) == it->second*sizeof(double));{
                fillDataFactory.at(convertFeatureToStorageType(type))->initfFigure(figure, std::move(points), it->second);
        }
        } catch(std::bad_alloc& ba)
        {
            return false;
	}

    }

    void draw() {
        if(figure) {
    	    figure->draw();
	}
    }

private: 
    typedef std::unique_ptr<double[]> Tpoints_ptr;

    class BaseFigure {
    public:
        BaseFigure(Tpoints_ptr aPoints, size_t aSize) : points(std::move(aPoints)), size_points(aSize) {} 

        virtual void draw() const = 0;

    protected:
        size_t size_points;
        Tpoints_ptr points;
    };

    class Circle : public BaseFigure {
    public:
        Circle(Tpoints_ptr aPoints, size_t aSize):BaseFigure(std::move(aPoints), aSize){}

        virtual void draw() const { 
	       drawCircle(points[0], points[1], points[2]);
	    }
    private:
        void drawCircle(double centerX, double centerY, double radius) const;
    };

    class Polygone : public BaseFigure {
    public:
        Polygone(Tpoints_ptr aPoints, size_t aSize):BaseFigure(std::move(aPoints), aSize){}
        virtual void draw() const { 
            drawPolygon(points.get()), size_points);
        }
    private:
        void drawPolygon(double* points, int size) const;
    };

    class DummyFactoryBase {
    public: 
        virtual void initfFigure(std::unique_ptr<BaseFigure>&, Tpoints_ptr, size_t) = 0;
    };

    template <typename T>
    class Factory : public DummyFactoryBase {	
    public:
        void initfFigure(std::unique_ptr<BaseFigure>& figure, Tpoints_ptr my_ptr, size_t size) { 
            figure.reset(new T(std::move(my_ptr), size));
     	}
    };

protected: 
    const std::map<FeatureType, size_t> figure_sizes {{eCircle,3} ,{eTriangle,6} , {eSquare,8}}; 
    std::map<StorageType, std::unique_ptr<DummyFactoryBase> > fillDataFactory;  
    std::unique_ptr<BaseFigure> figure;       
};
 
int main(int argc, char* argv[])
{
    Feature feature;
    FILE* file = fopen("features.dat", "r");
    feature.read(file);
    if (!feature.isValid())
        return 1;
    return 0;
}
