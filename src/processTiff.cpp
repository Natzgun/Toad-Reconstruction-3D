#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <tiffio.h>
#include <vector>

struct Point {
  int x, y, z;
};

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Uso: " << argv[0] << " archivo.tiff\n";
    return 1;
  }

  const char *filename = argv[1];

  TIFF *tif = TIFFOpen(filename, "r");
  if (!tif) {
    std::cerr << "No se pudo abrir el archivo.\n";
    return 1;
  }

  uint32_t width, height;
  uint32_t *raster;
  std::vector<Point> points;

  int z = 0;
  const int stride = 2;

  do {
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

    raster = (uint32_t *)_TIFFmalloc(width * height * sizeof(uint32_t));

    if (TIFFReadRGBAImage(tif, width, height, raster, 0)) {
      for (uint32_t y = 1; y < height - 1; y += stride) {
        for (uint32_t x = 1; x < width - 1; x += stride) {
          uint32_t pixel = raster[y * width + x];
          uint8_t r = TIFFGetR(pixel);

          if (r == 255) {
            // Filtrado de puntos frontera
            bool frontera = false;
            if (TIFFGetR(raster[y * width + (x - 1)]) == 0)
              frontera = true;
            if (TIFFGetR(raster[y * width + (x + 1)]) == 0)
              frontera = true;
            if (TIFFGetR(raster[(y - 1) * width + x]) == 0)
              frontera = true;
            if (TIFFGetR(raster[(y + 1) * width + x]) == 0)
              frontera = true;

            if (frontera) {
              points.push_back({(int)x, (int)(height - y - 1), z});
            }
          }
        }
      }
    }

    _TIFFfree(raster);
    z++;

  } while (TIFFReadDirectory(tif));

  TIFFClose(tif);

  std::string baseName(filename);
  size_t dotPos = baseName.find_last_of('.');
  if (dotPos != std::string::npos) {
    baseName = baseName.substr(0, dotPos);
  }

  std::ofstream obj(baseName + ".obj");
  for (const auto &p : points) {
    obj << "v " << p.x << " " << p.y << " " << p.z << "\n";
  }
  obj.close();

  std::cout << "Archivo " << baseName << ".obj generado con " << points.size()
            << " puntos.\n";
  return 0;
}
