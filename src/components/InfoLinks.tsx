
import { motion } from 'framer-motion';
import { FileText, Code, BookOpen, ExternalLink } from 'lucide-react';

const links = [
  {
    name: 'Paper',
    icon: FileText,
    href: '#',
  },
  {
    name: 'Code',
    icon: Code,
    href: '#',
  },
  {
    name: 'Wiki',
    icon: BookOpen,
    href: '#',
  },
  {
    name: 'Cite',
    icon: ExternalLink,
    href: '#',
  },
];

const InfoLinks = () => {
  return (
    <section className="py-10 px-6 md:px-12">
      <motion.div 
        className="max-w-5xl mx-auto flex flex-wrap justify-center gap-4"
        initial={{ opacity: 0, y: 20 }}
        animate={{ opacity: 1, y: 0 }}
        transition={{ duration: 0.5, delay: 0.3 }}
      >
        {links.map((link, index) => (
          <motion.a
            key={link.name}
            href={link.href}
            className="flex items-center space-x-2 px-5 py-3 rounded-full bg-white shadow-sm hover:shadow transition-all duration-200 border border-gray-100"
            whileHover={{ y: -2, backgroundColor: 'rgba(255, 255, 255, 0.9)' }}
            initial={{ opacity: 0, y: 10 }}
            animate={{ opacity: 1, y: 0 }}
            transition={{ duration: 0.3, delay: 0.2 + index * 0.1 }}
          >
            <link.icon className="h-5 w-5 text-tpg-blue" />
            <span className="text-sm font-medium">{link.name}</span>
          </motion.a>
        ))}
      </motion.div>
    </section>
  );
};

export default InfoLinks;
