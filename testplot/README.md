# TXT Data Plotter - Enhanced Version 2.0

A powerful Qt-based application for plotting and analyzing data from TXT files with multiple advanced chart types and statistical visualizations.

## 🆕 New Features (Version 2.0)

### Advanced Chart Types
- **📦 Box Plot**: Statistical summary with quartiles, median, and outliers
- **🎻 Violin Plot**: Kernel density estimation with distribution shape
- **🌊 Density Plot**: Smooth probability density curves
- **🏔️ Area Chart**: Enhanced line charts with gradient fills

### Command Line Interface
- Load files directly from command line
- Set chart type on startup
- Full help system with --help option

### Enhanced Visual Design
- Modern color palette with gradient effects
- Improved chart aesthetics with shadows and highlights
- Better typography and spacing
- Rounded corners and smooth animations

## 📊 Chart Types Available

### Basic Charts
- **📈 Line Chart**: Shows data trends with gradient fills and enhanced points
- **📊 Bar Chart**: 3D-style bars with gradients and highlights
- **🥧 Pie Chart**: Professional pie charts with leader lines and labels
- **🔸 Scatter Plot**: Enhanced scatter points with multiple colors
- **📋 Histogram**: Frequency distribution with smart binning

### Statistical Charts  
- **📦 Box Plot**: Five-number summary with outlier detection
- **🎻 Violin Plot**: Combines box plot with kernel density estimation
- **🌊 Density Plot**: Smooth probability density curves
- **🏔️ Area Chart**: Filled area under curves with multiple gradients

## 🚀 Command Line Usage

```bash
# Show help
txtplotter.exe --help

# Load file with specific chart type
txtplotter.exe --file data.txt --type violin

# Available chart types: line, bar, pie, scatter, histogram, box, violin, density, area
```

### Examples
```bash
# Load sample data as box plot
txtplotter.exe -f sample_data.txt -t box

# Load statistics data as density plot  
txtplotter.exe --file test_stats.txt --type density

# Load with area chart
txtplotter.exe -f data.txt -t area
```

## 🏗️ Building

### Prerequisites
- Qt5 or Qt6
- CMake 3.16+
- C++17 compatible compiler (MSVC, GCC, Clang)

### Build Instructions

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## 📖 Usage

### GUI Mode
1. Run `txtplotter.exe` 
2. Click "📂 Load TXT File" to load your data
3. Select chart type from the enhanced radio button panel
4. Customize chart title and axis labels
5. View detailed statistics in the left panel

### Command Line Mode
```bash
# Direct file loading with chart type
txtplotter.exe --file "path/to/data.txt" --type violin
```

## 📝 Data Format Support

**Single column** (Y values, X = indices):
```
23.5
18.2
31.7
29.1
15.6
```

**Two columns** (X and Y values):
```
1.0 2.5
2.0 4.1  
3.0 3.8
4.0 5.2
6.0 5.9
```

## 📈 Statistical Analysis

### Comprehensive Statistics Display
- **Basic**: Count, Sum, Mean, Median
- **Variability**: Standard Deviation, Variance
- **Range**: Min, Max, Range
- **Quartiles**: Q1, Q3, Interquartile Range (IQR)
- **Distribution**: Outlier detection for Box Plots

### Advanced Features
- **Box Plots**: Automatic outlier detection using 1.5×IQR rule
- **Violin Plots**: Gaussian kernel density estimation
- **Density Plots**: Adaptive bandwidth selection
- **Histogram**: Smart binning using square-root rule

## 🎨 Visual Enhancements

### Modern Design Elements
- **Gradient Fills**: Multi-stop gradients for all chart types
- **Shadow Effects**: Subtle drop shadows on UI elements
- **Rounded Corners**: Smooth, modern interface design
- **Color Harmony**: Carefully selected color palette
- **Typography**: Clear, readable fonts with proper hierarchy

### Chart-Specific Improvements
- **Line Charts**: Smooth gradients under curves
- **Bar Charts**: 3D-style highlighting and rounded corners
- **Pie Charts**: Professional leader lines and label boxes
- **Statistical Charts**: Clear quartile indicators and outlier highlighting

## 📁 Demo and Testing

Run the demo script to see all chart types:
```bash
demo.bat
```

This will open multiple windows showcasing:
- Line charts with sample time-series data
- Bar charts with categorical data
- Statistical visualizations with test data
- All chart types with different styling

## 🔧 Configuration

The application supports various customization options:
- Chart titles and axis labels
- Real-time chart type switching
- Responsive window resizing
- High-DPI display support

## 🐛 Troubleshooting

### Common Issues
- **Build Errors**: Ensure Qt development libraries are installed
- **Missing Data**: Check TXT file format (space or tab separated)
- **Display Issues**: Update graphics drivers for best rendering

### Platform-Specific Notes
- **Windows**: Requires Visual C++ Redistributable
- **Linux**: Requires Qt5/Qt6 development packages
- **macOS**: May need additional Xcode tools

## 🤝 Contributing

This enhanced version includes significant improvements in:
- Statistical visualization capabilities
- Command-line interface
- Visual design and user experience
- Code organization and maintainability

## 📄 Version History

- **v2.0**: Added statistical charts, CLI support, visual enhancements
- **v1.0**: Basic chart types with simple GUI interface