/**
 * ESP32 Distance Sensor - Shared JavaScript Functionality
 * Provides common functionality across all pages
 */

// Global configuration
const CONFIG = {
    refreshInterval: 1000,  // 1 second - faster updates for testing
    notificationDuration: 3000,  // 3 seconds
    apiTimeout: 10000  // 10 seconds
};

// Utility functions
function showNotification(message, type = 'info') {
    // Remove existing notifications
    const existing = document.querySelectorAll('.notification');
    existing.forEach(el => el.remove());
    
    // Create new notification
    const notification = document.createElement('div');
    notification.className = `notification ${type}`;
    notification.textContent = message;
    
    // Add to page
    document.body.appendChild(notification);
    
    // Auto-remove after duration
    setTimeout(() => {
        if (notification.parentNode) {
            notification.parentNode.removeChild(notification);
        }
    }, CONFIG.notificationDuration);
}

// Navigation helper
function navigateTo(page) {
    window.location.href = page;
}

// Active navigation highlighting
function updateActiveNav() {
    const currentPage = window.location.pathname.split('/').pop() || 'index.html';
    const navBtns = document.querySelectorAll('.nav-btn');
    
    navBtns.forEach(btn => {
        btn.classList.remove('active');
        if (btn.getAttribute('href').includes(currentPage)) {
            btn.classList.add('active');
        }
    });
}

// Format timestamp for display
function formatTimestamp(timestamp) {
    const date = new Date(timestamp);
    return date.toLocaleTimeString();
}

// Format uptime in human-readable format
function formatUptime(seconds) {
    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    const secs = seconds % 60;
    
    if (days > 0) {
        return `${days}d ${hours}h ${minutes}m`;
    } else if (hours > 0) {
        return `${hours}h ${minutes}m ${secs}s`;
    } else if (minutes > 0) {
        return `${minutes}m ${secs}s`;
    } else {
        return `${secs}s`;
    }
}

// API helper functions
async function apiCall(endpoint, options = {}) {
    const defaultOptions = {
        timeout: CONFIG.apiTimeout,
        headers: {
            'Content-Type': 'application/json'
        }
    };
    
    const mergedOptions = { ...defaultOptions, ...options };
    
    try {
        const controller = new AbortController();
        const timeoutId = setTimeout(() => controller.abort(), mergedOptions.timeout);
        
        const response = await fetch(endpoint, {
            ...mergedOptions,
            signal: controller.signal
        });
        
        clearTimeout(timeoutId);
        
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        return await response.json();
    } catch (error) {
        if (error.name === 'AbortError') {
            throw new Error('Request timeout');
        }
        throw error;
    }
}

// Dashboard-specific functions
async function refreshData() {
    // Show loading state
    const distanceValue = document.getElementById('distance-value');
    const distanceStatus = document.getElementById('distance-status');
    const lastUpdate = document.getElementById('last-update');
    
    // Skip loading state for smoother updates
    
    try {
        // Fetch real distance data from API
        const response = await fetch('/api/distance', {
            method: 'GET',
            headers: {
                'Content-Type': 'application/json'
            },
            timeout: CONFIG.apiTimeout
        });
        
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        const data = await response.json();
        
        // Update display with real data
        if (distanceValue) {
            if (data.status === 'ok') {
                distanceValue.textContent = `${data.distance_cm.toFixed(1)} cm`;
                distanceStatus.textContent = 'Active';
                distanceValue.className = 'distance-value success';
                distanceStatus.className = 'distance-status success';
            } else {
                // Handle error states
                let errorMessage;
                switch (data.status) {
                    case 'timeout':
                        errorMessage = 'Sensor Timeout';
                        break;
                    case 'out_of_range':
                        errorMessage = 'Out of Range';
                        break;
                    case 'no_echo':
                        errorMessage = 'No Echo';
                        break;
                    case 'invalid':
                        errorMessage = 'Invalid Reading';
                        break;
                    default:
                        errorMessage = 'Sensor Error';
                }
                
                distanceValue.textContent = '-- cm';
                distanceStatus.textContent = errorMessage;
                distanceValue.className = 'distance-value error';
                distanceStatus.className = 'distance-status error';
            }
        }
        
        if (lastUpdate) {
            lastUpdate.textContent = formatTimestamp(Date.now());
        }
        
        // Remove the annoying success notification - only show errors
        
    } catch (error) {
        console.error('Failed to fetch distance data:', error);
        
        if (distanceValue) {
            distanceValue.textContent = '-- cm';
            distanceStatus.textContent = 'Connection Error';
            distanceValue.className = 'distance-value error';
            distanceStatus.className = 'distance-status error';
        }
        
        // Only show notification for errors
        showNotification('Failed to refresh data: ' + error.message, 'error');
    }
}

// Auto-refresh functionality for dashboard
let autoRefreshInterval = null;

function startAutoRefresh() {
    if (window.location.pathname.includes('index.html') || window.location.pathname === '/') {
        autoRefreshInterval = setInterval(refreshData, CONFIG.refreshInterval);
    }
}

function stopAutoRefresh() {
    if (autoRefreshInterval) {
        clearInterval(autoRefreshInterval);
        autoRefreshInterval = null;
    }
}

// Page-specific initialization
function initializePage() {
    updateActiveNav();
    
    const currentPage = window.location.pathname.split('/').pop() || 'index.html';
    
    switch (currentPage) {
        case 'index.html':
        case '':
            // Dashboard initialization
            if (typeof refreshData === 'function') {
                refreshData();
                startAutoRefresh();
            }
            break;
            
        case 'wifi-setup.html':
            // WiFi setup is handled by inline scripts
            break;
            
        case 'settings.html':
            // Settings page initialization
            break;
    }
}

// Error handling
window.addEventListener('error', function(event) {
    console.error('JavaScript error:', event.error);
    showNotification('An error occurred. Please refresh the page.', 'error');
});

// Unhandled promise rejection handling
window.addEventListener('unhandledrejection', function(event) {
    console.error('Unhandled promise rejection:', event.reason);
    showNotification('A network error occurred. Please try again.', 'error');
});

// Network status detection
window.addEventListener('online', function() {
    // Connection restored silently - no popup needed
    console.log('Connection restored');
});

window.addEventListener('offline', function() {
    showNotification('Connection lost', 'error');
    stopAutoRefresh();
});

// Page visibility API for auto-refresh management
document.addEventListener('visibilitychange', function() {
    if (document.hidden) {
        stopAutoRefresh();
    } else {
        startAutoRefresh();
    }
});

// Keyboard shortcuts
document.addEventListener('keydown', function(event) {
    // Ctrl/Cmd + R for refresh
    if ((event.ctrlKey || event.metaKey) && event.key === 'r') {
        event.preventDefault();
        if (typeof refreshData === 'function') {
            refreshData();
        }
    }
    
    // Numbers 1-3 for navigation
    if (event.altKey) {
        switch (event.key) {
            case '1':
                navigateTo('/index.html');
                break;
            case '2':
                navigateTo('/wifi-setup.html');
                break;
            case '3':
                navigateTo('/settings.html');
                break;
        }
    }
});

// Mobile menu handling (for future enhancement)
function toggleMobileMenu() {
    const navLinks = document.querySelector('.nav-links');
    navLinks.classList.toggle('mobile-open');
}

// Touch gesture support for mobile navigation
let touchStartX = 0;
let touchEndX = 0;

document.addEventListener('touchstart', function(event) {
    touchStartX = event.changedTouches[0].screenX;
});

document.addEventListener('touchend', function(event) {
    touchEndX = event.changedTouches[0].screenX;
    handleGesture();
});

function handleGesture() {
    const threshold = 100; // Minimum distance for swipe
    const difference = touchStartX - touchEndX;
    
    if (Math.abs(difference) > threshold) {
        const currentPage = window.location.pathname.split('/').pop() || 'index.html';
        
        if (difference > 0) {
            // Swipe left - next page
            switch (currentPage) {
                case 'index.html':
                    navigateTo('/wifi-setup.html');
                    break;
                case 'wifi-setup.html':
                    navigateTo('/settings.html');
                    break;
            }
        } else {
            // Swipe right - previous page
            switch (currentPage) {
                case 'settings.html':
                    navigateTo('/wifi-setup.html');
                    break;
                case 'wifi-setup.html':
                    navigateTo('/index.html');
                    break;
            }
        }
    }
}

// Performance monitoring
const performanceObserver = new PerformanceObserver((list) => {
    for (const entry of list.getEntries()) {
        if (entry.entryType === 'navigation') {
            console.log(`Page load time: ${entry.loadEventEnd - entry.loadEventStart}ms`);
        }
    }
});

if ('PerformanceObserver' in window) {
    performanceObserver.observe({ entryTypes: ['navigation'] });
}

// Initialize when DOM is loaded
document.addEventListener('DOMContentLoaded', initializePage);

// Cleanup on page unload
window.addEventListener('beforeunload', function() {
    stopAutoRefresh();
    
    if ('PerformanceObserver' in window) {
        performanceObserver.disconnect();
    }
});

// Export functions for global access
window.ESP32_DISTANCE = {
    refreshData,
    showNotification,
    navigateTo,
    apiCall,
    formatTimestamp,
    formatUptime
};