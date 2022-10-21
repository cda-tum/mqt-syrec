try:
    from importlib import metadata
except ImportError:
    import importlib_metadata as metadata

import pybtex.plugin
from pybtex.style.formatting.unsrt import Style as UnsrtStyle
from pybtex.style.template import field, href

# -- Project information -----------------------------------------------------
project = "SyReC"
author = "Smaran Adarsh"

release = metadata.version("mqt.syrec")
version = ".".join(release.split(".")[:3])
language = "en"
copyright = "Chair for Design Automation, Technical University of Munich"

# -- General configuration ---------------------------------------------------
extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.autosectionlabel",
    "sphinx.ext.intersphinx",
    "sphinx.ext.autosummary",
    "sphinx.ext.mathjax",
    "sphinx.ext.napoleon",
    "sphinx.ext.viewcode",
    "sphinx.ext.githubpages",
    "sphinxcontrib.bibtex",
    "sphinx_copybutton",
    "hoverxref.extension",
]

autosectionlabel_prefix_document = True

hoverxref_auto_ref = True
hoverxref_domains = ["cite", "py"]
hoverxref_roles = []
hoverxref_mathjax = True
hoverxref_role_types = {
    "ref": "tooltip",
    "p": "tooltip",
    "labelpar": "tooltip",
    "class": "tooltip",
    "meth": "tooltip",
    "func": "tooltip",
    "attr": "tooltip",
    "property": "tooltip",
}
exclude_patterns = ["_build", "build", "**.ipynb_checkpoints", "Thumbs.db", ".DS_Store", ".env"]


class CDAStyle(UnsrtStyle):
    def format_url(self, e):
        url = field("url", raw=True)
        return href()[url, "[PDF]"]


pybtex.plugin.register_plugin("pybtex.style.formatting", "cda_style", CDAStyle)

bibtex_bibfiles = ["refs.bib"]
bibtex_default_style = "cda_style"

copybutton_prompt_text = r"(?:\(venv\) )?\$ "
copybutton_prompt_is_regexp = True
copybutton_line_continuation_character = "\\"

autosummary_generate = True

# -- Options for HTML output -------------------------------------------------
html_theme = "sphinx_rtd_theme"
html_baseurl = "https://syrec.readthedocs.io/en/latest/"
autodoc_member_order = "groupwise"
