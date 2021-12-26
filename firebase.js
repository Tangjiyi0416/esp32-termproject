import {
    getDatabase,
    ref,
    set
} from "https://www.gstatic.com/firebasejs/9.6.1/firebase-database.js";
import { initializeApp } from "https://www.gstatic.com/firebasejs/9.6.1/firebase-app.js";
const firebaseConfig = {
    apiKey: "AIzaSyBheQiegwSHSxObaMvDSRc7nMpG2rQKQfY",
    authDomain: "esp32-term.firebaseapp.com",
    databaseURL: "https://esp32-term-default-rtdb.asia-southeast1.firebasedatabase.app",
    projectId: "esp32-term",
    storageBucket: "esp32-term.appspot.com",
    messagingSenderId: "459781635323",
    appId: "1:459781635323:web:fcb697e551b3543a23c45e",
    measurementId: "G-Z1VWQH96HL"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);