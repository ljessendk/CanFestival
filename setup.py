import os,sys
from distutils.core import setup


install_dir=os.path.join("LOLITech","CanFestival-3")

data_files=[]
os.getcwd()
os.chdir(os.getcwd())

def generate(base_dir):
    listfile=[]
    if base_dir == "":
        directory = "."
    else:
        directory = base_dir
    data_files.append((os.path.join(install_dir, base_dir), listfile))

    for element in os.listdir(directory):
        element_path=os.path.join(base_dir, element)
        if os.path.isdir(element_path):
            generate(element_path)
        elif os.path.isfile(element_path):
            ext_element=os.path.splitext(element)
            if ext_element[1] != ".o" and ext_element[1] != ".pyc":
                listfile.append(element_path)

generate("")


setup(name='CanFestival-3', # Name of the executable
      version='0.1', # Version
      description='Open-Source CanOpen Stack', #description
      author='Edouard Tisserant, Laurent Bessard',
      author_email='edouard.tisserant.lolitech.fr, laurent.bessard@lolitech.fr',
      url='http://www.canfestival.org',
      license='GPL',
      scripts=['objdictedit_postinst.py'],
      data_files=data_files, # Add files to install
)