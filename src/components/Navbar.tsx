
import { useState, useEffect } from 'react';
import { Link, useLocation } from 'react-router-dom';
import { cn } from '@/lib/utils';

const Navbar = () => {
  const [scrolled, setScrolled] = useState(false);
  const location = useLocation();
  
  useEffect(() => {
    const handleScroll = () => {
      if (window.scrollY > 20) {
        setScrolled(true);
      } else {
        setScrolled(false);
      }
    };
    
    window.addEventListener('scroll', handleScroll);
    return () => window.removeEventListener('scroll', handleScroll);
  }, []);

  return (
    <header 
      className={cn(
        "fixed top-0 left-0 right-0 z-50 transition-all duration-300 ease-in-out py-4 px-6 md:px-12",
        scrolled ? "bg-white/80 backdrop-blur-md shadow-sm" : "bg-transparent"
      )}
    >
      <div className="max-w-7xl mx-auto flex items-center justify-between">
        <Link 
          to="/" 
          className="text-lg font-medium transition-opacity hover:opacity-80"
        >
          TPG Playground
        </Link>
        <nav className="flex items-center space-x-8">
          <NavLink to="/" active={location.pathname === "/"}>
            Home
          </NavLink>
          <NavLink to="/experiments" active={location.pathname === "/experiments"}>
            Experiments
          </NavLink>
        </nav>
      </div>
    </header>
  );
};

interface NavLinkProps {
  to: string;
  active: boolean;
  children: React.ReactNode;
}

const NavLink = ({ to, active, children }: NavLinkProps) => {
  return (
    <Link
      to={to}
      className={cn(
        "relative py-1 text-sm font-medium transition-colors",
        active ? "text-tpg-blue" : "text-gray-700 hover:text-tpg-blue"
      )}
    >
      {children}
      <span 
        className={cn(
          "absolute bottom-0 left-0 h-0.5 bg-tpg-blue transition-all duration-300",
          active ? "w-full" : "w-0"
        )} 
      />
    </Link>
  );
};

export default Navbar;
