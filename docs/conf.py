# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'ESP32 Template'
copyright = '2025, ESP32 Template Team'
author = 'ESP32 Template Team'
release = '1.0.0'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.intersphinx',
    'sphinx.ext.todo',
    'sphinx.ext.viewcode',
    'sphinx.ext.graphviz',
    'sphinx_needs',
    'sphinxcontrib.plantuml',
    'myst_parser',
]

templates_path = ['_templates']
exclude_patterns = [
    '_build',
    'Thumbs.db',
    '.DS_Store',
    '01_general',  # Empty placeholder folders
    '02_development',
]

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'
html_static_path = ['_static']
html_title = 'ESP32 Template Documentation'

# Custom CSS for dark mode fixes
html_css_files = [
    'custom.css',
]

# -- Sphinx-Needs Configuration ----------------------------------------------
# https://sphinx-needs.readthedocs.io/

needs_types = [
    dict(
        directive="req",
        title="Requirement",
        prefix="REQ_",
        color="#BFD8D2",
        style="node"
    ),
    dict(
        directive="spec",
        title="Design Specification",
        prefix="DSN_",
        color="#FEDCD2",
        style="node"
    ),
    dict(
        directive="impl",
        title="Implementation",
        prefix="IMPL_",
        color="#DF744A",
        style="node"
    ),
    dict(
        directive="test",
        title="Test Case",
        prefix="TEST_",
        color="#DCB239",
        style="node"
    ),
]

# Extra options for needs (status is built-in, don't redefine it)
needs_extra_options = [
    "priority",
    "rationale",
    "acceptance_criteria",
]

# Status options
needs_statuses = [
    dict(name="draft", description="Draft - Work in progress"),
    dict(name="approved", description="Approved - Ready for implementation"),
    dict(name="implemented", description="Implemented - Code exists"),
    dict(name="verified", description="Verified - Tested and validated"),
    dict(name="deprecated", description="Deprecated - No longer used"),
]

# Priority options
needs_priority = [
    dict(name="mandatory", description="Must have - Critical requirement"),
    dict(name="high", description="Should have - Important requirement"),
    dict(name="medium", description="Could have - Nice to have"),
    dict(name="low", description="Won't have this time - Future consideration"),
]

# Enable automatic ID generation
needs_id_required = True

# Configure needs file output
needs_build_json = True
needs_build_json_per_id = True

# Use Graphviz for needflow diagrams instead of PlantUML
needs_flow_engine = "graphviz"

# -- MyST Parser Configuration -----------------------------------------------
# https://myst-parser.readthedocs.io/

myst_enable_extensions = [
    "colon_fence",
    "deflist",
    "html_image",
]

# -- Intersphinx Configuration -----------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/extensions/intersphinx.html

intersphinx_mapping = {
    'python': ('https://docs.python.org/3', None),
}

# -- Todo Extension Configuration --------------------------------------------

todo_include_todos = True
