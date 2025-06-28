# Feature: Implement Basic Static Web Interface with Navbar Navigation

## Overview
Implement a modern, mobile-responsive single-page web application with navbar navigation to replace the current basic captive portal. The interface should provide dashboard and settings sections with build-time embedded assets served from ESP32 flash memory.

## Current State
- ✅ Basic HTTP server and WiFi management implemented (`main/web_server.c`, `main/wifi_manager.c`)
- ✅ Simple captive portal functionality working
- ✅ Distance sensor API available: `distance_sensor_get_latest()`
- ✅ Display logic providing real-time LED visualization

## Requirements

### 1. File Structure Implementation
Create a new web assets directory structure:
```
main/www/
├── index.html          # Main dashboard
├── wifi-setup.html     # WiFi configuration (current captive portal)
├── settings.html       # System configuration  
├── css/
│   └── style.css       # Shared styles
└── js/
    └── app.js          # Shared JavaScript functionality
```

### 2. Multi-Page HTML Structure

- **Separate HTML Files**: Individual pages for each section with shared navigation
- **Navbar Navigation**: Consistent navigation bar across all pages  
- **Responsive Design**: Mobile-first approach with touch-friendly interface
- **Content Pages**:
  - **index.html**: Main dashboard with distance display
  - **wifi-setup.html**: WiFi configuration (preserve existing captive portal)
  - **settings.html**: System configuration with project link in footer

**Expected HTML Structure (index.html example)**:
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Distance Sensor - Dashboard</title>
    <link rel="stylesheet" href="/css/style.css">
</head>
<body>
    <nav class="navbar">
        <div class="nav-brand">Distance Sensor</div>
        <div class="nav-links">
            <a href="/index.html" class="nav-btn active">Dashboard</a>
            <a href="/wifi-setup.html" class="nav-btn">WiFi</a>
            <a href="/settings.html" class="nav-btn">Settings</a>
        </div>
    </nav>
    
    <main class="container">
        <h2>Dashboard</h2>
        <div class="distance-display">
            <span class="distance-value" id="distance-value">-- cm</span>
            <span class="distance-status" id="distance-status">Loading...</span>
        </div>
        <button onclick="refreshData()" class="refresh-btn">Refresh</button>
    </main>
    
    <script src="/js/app.js"></script>
</body>
</html>
```

### 3. CSS Styling (`main/www/style.css`)
- **Mobile-First Design**: Responsive breakpoints for tablet/desktop
- **Modern UI**: Clean, minimalist design with good contrast
- **Touch-Friendly**: Minimum 44px touch targets for mobile
- **Color Scheme**: Professional blue/gray theme with accent colors

**Key CSS Features**:
```css
/* Mobile-first responsive design */
* { box-sizing: border-box; }

/* Fixed navbar */
.navbar {
    position: fixed;
    top: 0;
    width: 100%;
    background: #2c3e50;
    z-index: 1000;
}

/* Touch-friendly navigation */
.nav-btn {
    min-height: 44px;
    padding: 12px 20px;
    font-size: 16px;
}

/* Content with navbar offset */
.container {
    margin-top: 60px;
    padding: 20px;
}

/* Distance display styling */
.distance-display {
    text-align: center;
    margin: 40px 0;
}

.distance-value {
    font-size: 3rem;
    font-weight: bold;
    color: #27ae60;
}

/* Responsive breakpoints */
@media (min-width: 768px) {
    .nav-links { display: flex; }
    .container { max-width: 800px; margin: 60px auto 0; }
}
```

### 4. JavaScript Functionality (`main/www/app.js`)
- **Navbar Navigation**: Switch between Dashboard and Settings sections
- **Data Fetching**: Get current distance from existing sensor API
- **UI Updates**: Refresh distance display and system status
- **Error Handling**: Display connection/sensor errors appropriately

**Expected JavaScript Features**:
```javascript
// Navigation handling
document.addEventListener('DOMContentLoaded', function() {
    setupNavigation();
    loadDashboardData();
});

function setupNavigation() {
    const navButtons = document.querySelectorAll('.nav-btn');
    const sections = document.querySelectorAll('.content-section');
    
    navButtons.forEach(button => {
        button.addEventListener('click', () => {
            const targetSection = button.getAttribute('data-section');
            showSection(targetSection);
        });
    });
}

function showSection(sectionId) {
    // Hide all sections, show target
    // Update active navbar button
}

async function refreshData() {
    // Phase 1: Static placeholder data
    updateDistanceDisplay({
        distance: "25.4",
        status: "Demo Mode"
    });
    
    // TODO: Phase 2 - Replace with real API call:
    // const response = await fetch('/api/distance');
    // const data = await response.json();
}
```

### 5. ESP-IDF Integration

#### A. CMakeLists.txt Updates (`main/CMakeLists.txt`)
Add embedded file generation for all web assets:
```cmake
# Embed HTML pages
target_add_binary_data(${COMPONENT_LIB} "www/index.html" TEXT)
target_add_binary_data(${COMPONENT_LIB} "www/wifi-setup.html" TEXT)
target_add_binary_data(${COMPONENT_LIB} "www/settings.html" TEXT)

# Embed CSS and JavaScript
target_add_binary_data(${COMPONENT_LIB} "www/css/style.css" TEXT)
target_add_binary_data(${COMPONENT_LIB} "www/js/app.js" TEXT)
```

#### B. Web Server Updates (`main/web_server.c`)

- **Static File Handler**: Serve embedded HTML/CSS/JS files with proper MIME types
- **Root Redirect**: Redirect `/` to serve the main application (`/app.html`)  
- **Content-Type Headers**: `text/html`, `text/css`, `application/javascript`
- **Embedded File Access**: Use ESP-IDF generated symbols to access flash-stored files

**Required Handler Functions**:

```c
// Serve embedded static files
esp_err_t static_file_handler(httpd_req_t *req);

// Helper: Get embedded file content and size
const char* get_embedded_file(const char* filename, size_t* size);

// Helper: Get MIME type from file extension  
const char* get_mime_type(const char* filename);
```

### 6. Static File Serving (No API Yet)

- **Embedded Files**: HTML/CSS/JS compiled into ESP32 flash at build time
- **Static Data**: Dashboard shows placeholder distance values for now
- **File Serving**: Serve embedded assets via HTTP with proper MIME types
- **Root Redirect**: Redirect `/` to serve the main application page

**Note**: Real distance API integration will be added in Step 4.3 (Configuration Management & Data Sharing)

### 7. Mobile Responsiveness Requirements
- **Viewport Meta**: Proper mobile viewport configuration
- **Touch Targets**: Minimum 44px for all interactive elements
- **Responsive Layout**: Single column on mobile, flexible on desktop
- **Fast Loading**: Optimized asset sizes for ESP32 memory constraints

## Technical Specifications

### Memory Constraints
- **Total Assets Size**: Target < 20KB for all HTML/CSS/JS combined
- **Flash Embedding**: Use ESP-IDF `EMBED_FILES` for build-time inclusion
- **Compression**: Consider gzip compression for larger assets

### Browser Compatibility
- **Modern Browsers**: Chrome, Firefox, Safari, Edge (last 2 versions)
- **Mobile Support**: iOS Safari, Android Chrome
- **Progressive Enhancement**: Core functionality without JavaScript

### Performance Goals
- **Page Load**: < 2 seconds on WiFi connection
- **Navigation**: Instant section switching (no page reloads)
- **Data Refresh**: < 1 second for distance updates

## Testing Requirements

### Functional Testing
- [ ] Navbar navigation switches sections correctly
- [ ] Dashboard displays current distance measurement
- [ ] Settings shows system information
- [ ] Refresh button updates distance data
- [ ] Mobile touch interaction works properly

### Integration Testing  
- [ ] Static files served correctly from flash memory
- [ ] API endpoints return proper JSON responses
- [ ] Error handling displays appropriate messages
- [ ] WiFi captive portal still functions

### Cross-Device Testing
- [ ] Responsive layout on mobile phones (320-480px)
- [ ] Tablet layout optimization (768-1024px)  
- [ ] Desktop display (1200px+)
- [ ] Touch and mouse interaction compatibility

## Implementation Priority

### Phase 1: Core Structure
1. Create `main/www/` directory and basic files
2. Implement HTML navbar structure
3. Add CSS mobile-responsive styling
4. Create basic JavaScript navigation

### Phase 2: ESP-IDF Integration
1. Update CMakeLists.txt for embedded assets
2. Modify web_server.c to serve static files
3. Add API endpoint for distance data
4. Test file serving and navigation

### Phase 3: Data Integration
1. Connect distance sensor API to web interface
2. Implement real-time data refresh
3. Add system status information
4. Test end-to-end functionality

## Success Criteria
- ✅ Modern, professional web interface accessible via ESP32 IP
- ✅ Mobile-responsive design works on smartphones and tablets  
- ✅ Navbar navigation provides smooth section switching
- ✅ Dashboard displays placeholder distance values (real API in Step 4.3)
- ✅ Settings section shows system information and links
- ✅ All assets served from ESP32 flash memory (no external dependencies)
- ✅ WiFi captive portal functionality preserved
- ✅ Page loads quickly and performs well on mobile devices

## Files to Modify/Create
- `main/www/index.html` (new - main dashboard)
- `main/www/wifi-setup.html` (new - WiFi configuration)  
- `main/www/settings.html` (new - system settings with project link)
- `main/www/css/style.css` (new - shared styles)
- `main/www/js/app.js` (new - shared JavaScript)
- `main/CMakeLists.txt` (modify - add embedded assets)
- `main/web_server.c` (modify - add static file serving)
- `main/web_server.h` (modify - add new handler declarations)

## Related Documentation
- Update IMPLEMENTATION_PLAN.md Step 4.2 status upon completion
- Document API endpoints in ARCHITECTURE.md
- Add usage instructions to README.md

---

**Assignee**: @github-copilot  
**Priority**: High  
**Labels**: enhancement, web-interface, static-assets, mobile-responsive
